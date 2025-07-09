#include "simd_gaussian_elimination.h"

#define SENTINEL (-1ll) 

void debug_print_block(uint64_t block[64])
{
    for (size_t i = 0; i < 64; i++)
    {
        printf("-");
    }
    printf("\n");

    for (size_t i = 0; i < 64; i++)
    {
        printf("|");
        for (size_t j = 0; j < 64; j++)
        {
           printf("%d", !!(block[j] & (1ull << i))); 
        }
        printf("|\n");
    }
    for (size_t i = 0; i < 64; i++)
    {
        printf("-");
    }
    printf("\n");
}

void debug_print_chunk(uint64_t chunk)
{
    printf("|");
    for (size_t i = 0; i < 64; i++)
    {
        printf("%d", !!(chunk & (1ull << i))); 
    }
    printf("|\n");
}


void zero_z_diagonal(widget_t* wid);
void zero_phases(widget_t* wid);

/*
 * simd_widget_decompose
 * Decomposes the stabiliser tableau into a graph state plus local Cliffords
 * :: wid : widget_t* :: Widget to decompose
 * Acts in place on the tableau
 */
void simd_widget_decompose(widget_t* wid)
{
    // Remove zero X columns
    // It's faster to do this prior to transposing as Hadamards are
    // Cache line aligned at this point 
    tableau_remove_zero_X_columns(
        wid->tableau,
        wid->queue
    );

    // Transpose the tableau for aligned rowsum operations 

    tableau_transpose(wid->tableau);

    // Perform the elimination
    simd_tableau_elim(wid);

    // Zero the Z diagonal
    zero_z_diagonal(wid);

    // Zero phases
    zero_phases(wid);

    return;
}


/*
 * naive_tableau_elim_upper
 * naive_tableau_elim_lower
 * 
 * Performs the upper and lower eliminations 
 *
 * :: wid : widget_t* :: Widget object 
 *
 */ 
void tableau_elim_upper(widget_t* wid)
{
    for (size_t i = 0; i < wid->tableau->n_qubits; i++)
    {
        if (0 == __inline_slice_get_bit(
            wid->tableau->slices_x[i], i)
           )
        {
            tableau_X_diag_element(
                wid->tableau,
                wid->queue, i);
        }
        tableau_X_diag_col_upper(wid->tableau, i);
    }
}
void tableau_elim_lower(widget_t* wid)
{
    for (size_t i = 0; i < wid->tableau->n_qubits; i++)
    {
        if (0 == __inline_slice_get_bit(wid->tableau->slices_x[i], i))
        {
            simd_tableau_X_diag_element(wid->tableau, wid->queue, i);
        }
        tableau_X_diag_col_lower(wid->tableau, i);
    }
}

/*
 * naive_widget_decompose
 * Decomposes the stabiliser tableau into a graph state plus local Cliffords
 * :: wid : widget_t* :: Widget to decompose
 * Acts in place on the tableau
 */
void naive_widget_decompose(widget_t* wid)
{
    tableau_remove_zero_X_columns(
        wid->tableau,
        wid->queue
    );

    tableau_transpose(wid->tableau);

    tableau_elim_upper(wid);

    tableau_elim_lower(wid);

    zero_z_diagonal(wid);

    zero_phases(wid);

    return;
}

#define BLOCK_STRIDE_BITS 64
/*
 * Loads a tile from memory
 * TODO: rename to tile
 * TODO: Comment blocks 
 */
static inline
void __inline_decomp_load_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
)
{
        #pragma GCC unroll 64 
        for (size_t j = 0; j < BLOCK_STRIDE_BITS; j++)
        {
           block[j] = GET_CHUNK(slices, slice_len, row_offset, col_offset + j);
        }
}
void decomp_load_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
)
{
    __inline_decomp_load_block(block, slices, slice_len, row_offset, col_offset);
}

/*
 * Stores a block in memory
 * Primarily used for testing the load block function
 */
static inline
void __inline_decomp_store_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
)
{
        #pragma GCC unroll 64 
        for (size_t j = 0; j < BLOCK_STRIDE_BITS; j++)
        {
           *(uint64_t*)(
                slices  
                + slice_len * (col_offset + j) 
                + row_offset / 8) = block[j];
        }
}
void decomp_store_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
)
{
    __inline_decomp_store_block(block, slices, slice_len, row_offset, col_offset);
}

/*
 * Performs an elimination on the upper portion of the local block
 * :: wid : widget_t* :: Widget object 
 * :: offset : size_t :: Offset of the tile
 * :: start : size_t :: Start of the decomp
 * :: end : size_t :: End of the decomp
 * :: ctrl_block : uint64_t[64] :: Tile
 */
static inline
void decomp_local_elim_upper(
        widget_t* wid,
        const size_t offset,
        const size_t start,
        const size_t end,
        uint64_t ctrl_block[64])
{
    size_t ctrl = 0;
    size_t block_end = 64;

    // Clean up to the target 
    for (size_t i = start; i < end; i++)
    {
        while (
        i > 
        (ctrl = __builtin_ctzll(ctrl_block[i])))
        {
            DPRINT(
                DEBUG_3, 
                "Upper Elim on %zu -> %zu\n", 
                i, ctrl);

            ctrl_block[i] ^= ctrl_block[ctrl];
            tableau_rowsum_offset(
                    wid->tableau,
                    ctrl + offset,
                    i + offset,
                    offset);
        }
    }

    // Clean up to the end 
    for (size_t i = end; i < block_end; i++)
    {
        while (
        end > 
        (ctrl = __builtin_ctzll(ctrl_block[i])))
        {
            ctrl_block[i] ^= ctrl_block[ctrl];

            tableau_rowsum_offset(
                    wid->tableau,
                    ctrl + offset,
                    i + offset,
                    offset); 
        }
    }
}

/*
 * Performs an elimination on the lower portion of the local block
 * :: wid : widget_t* :: Widget object 
 * :: offset : size_t :: Offset of the tile
 * :: start : size_t :: Start of the decomp
 * :: end : size_t :: End of the decomp
 * :: ctrl_block : uint64_t[64] :: Tile
 */
static inline
void decomp_local_elim_lower(
        widget_t* wid,
        const size_t offset,
        const size_t start,
        const size_t end,
        uint64_t ctrl_block[64])
{

    size_t ctrl = 0;
    uint64_t mask = 0;

    // Avoid undefined behaviour for right shifts at or beyond the size of 
    // mask TODO - replace branch while still avoiding UB?
    if (end < 64)
    {
        mask = ((1ull << end) - 1ull);
    }
    else
    {
        mask = -1;
    }

    // If end is 64 then mask is zeroed
    // This operation avoids that in a branch-free manner
    mask = (!!(mask) * mask) | (!(mask) * (~mask));

    // Clean up to the target 
    for (size_t i = 0; i < end; i++)
    {
        // Optimising compiler notices that this should always be bounded 
        // Problem is that it can throw INT_MAX as an output and overflow
        ctrl = 63 - __builtin_clzll(mask & ctrl_block[i]);

        while ((mask & ctrl_block[i]) && (ctrl > i))
        {

            DPRINT(
                DEBUG_3, 
                "Lower Elim on %zu -> %zu\n", 
                i, ctrl);

            ctrl_block[i] ^= ctrl_block[ctrl];
            tableau_rowsum_offset(
                    wid->tableau,
                    ctrl + offset,
                    i + offset,
                    offset);

            ctrl = 63 - __builtin_clzll(mask & ctrl_block[i]);

        }
    }
}

/*
 * decomp_local_elim 
 * Wrapper around the upper and lower calls
 */
static inline
void decomp_local_elim(
        widget_t* wid,
        const size_t offset,
        const size_t start,
        const size_t end,
        uint64_t ctrl_block[64])

{
    DPRINT(DEBUG_3, "Performing Local elim on block: %zu from %zu to %zu\n", 
            offset, start, end);

    decomp_local_elim_upper(wid, offset, start, end, ctrl_block);
    decomp_local_elim_lower(wid, offset, start, end, ctrl_block);
}

/*
 * Performs a local search
 * Attempts to find a valid index on the control block
 */
static inline
size_t decomp_local_search(
        widget_t* wid,
        const size_t offset,
        const size_t idx,
        uint64_t ctrl_block[64])
{
    const uint64_t mask = (1ull << idx);
    for (size_t i = idx + 1; i < 64; i++)
    {
        if (mask & ctrl_block[i]) 
        {
            uint64_t tmp = ctrl_block[idx]; 
            ctrl_block[idx] = ctrl_block[i]; 
            ctrl_block[i] = tmp; 

            simd_swap(
                    ((void*)wid->tableau->slices_x[0]) + wid->tableau->slice_len * (offset + idx), 
                    ((void*)wid->tableau->slices_z[0]) + wid->tableau->slice_len * (offset + idx), 
                    ((void*)wid->tableau->slices_x[0]) + wid->tableau->slice_len * (offset + i), 
                    ((void*)wid->tableau->slices_z[0]) + wid->tableau->slice_len * (offset + i), 
                    wid->tableau->slice_len
                    );

            return i; 
        }
    } 
    return SENTINEL;
}

/*
 * Performs a search and progressive elimination as one step 
 * This search is non-local, it begins on the tile subsequent to the diagonal 
 * TODO: comment header
 */
static inline
size_t decomp_non_local_search_and_elim(
    widget_t* wid,
    void* slices,
    const size_t slice_len_bytes,
    const size_t offset,
    const size_t index)
{
    const uint64_t mask = (1ull << index);
    const size_t column = offset / 64;
    const size_t block_end = (wid->tableau->n_qubits / 64) * 64;
    const size_t block_tail = 64 - (wid->tableau->n_qubits % 64);

    // Triggers if block is very small
    if (__builtin_expect(block_end == 0, 0))
    {
        return SENTINEL;
    }

    uint64_t targ_block[64];

    uint64_t ctrl_block[64];

    for (size_t i = offset + 64; i < block_end; i += 64) 
    {
        uint64_t ctrl;
        
        decomp_load_block(targ_block, slices, slice_len_bytes, offset, i);

        #pragma GCC unroll 64 
        for (size_t j = 0; j < 64; j++)
        {
            // While not all bits unset
            while ((ctrl = __builtin_ctzll(targ_block[j])) < index)
            {

                // Rowsum to unset bit
                tableau_rowsum_offset(
                    wid->tableau,
                    ctrl + offset,
                    i + j,
                    offset); 
                
                // Reload block
                targ_block[j] = GET_CHUNK(slices, slice_len_bytes, offset, i + j); 
            }

            if (__builtin_expect(targ_block[j] & mask, 0))
            {
                return i + j; 
            }
        }
    }
    return SENTINEL;
}
 
/*
 * Searches the Z tableau 
 */
static inline
size_t decomp_z_search(
    widget_t* wid,
    void* slices,
    const size_t slice_len_bytes,
    const size_t offset,
    const size_t index)
{
    const uint64_t mask = (1ull << index);
    uint64_t targ_block[64];

    decomp_load_block(targ_block, slices, slice_len_bytes, offset, offset);

    // Search local block
    for (size_t j = index + 1; j < 64; j++)
    {
        if (targ_block[j] & mask)
        {
            return offset + j; 
        }
    } 

    // Search non-local blocks
    for (size_t i = offset + 64; i < slice_len_bytes * 8; i += 64) 
    {
        decomp_load_block(targ_block, slices, slice_len_bytes, offset, i);

        //#pragma GCC unroll 64 
        for (size_t j = 0; j < 64; j++)
        {
            if (__builtin_expect(targ_block[j] & mask, 0))
            {
                return i + j; 
            }
        }
    }
    return SENTINEL;
}


/*
 * Zeros block if previous elements are set 
 * This search is non-local, it begins on the tile subsequent to the diagonal 
 * TODO: comment header
 */
static inline
void decomp_col_elim_upper(
    widget_t* wid,
    void* slices,
    const size_t slice_len_bytes,
    const size_t offset
    )
{
    for (size_t i = offset + 64; i < wid->tableau->n_qubits; i += 64) 
    {
        uint64_t targ_block[64];
        uint64_t ctrl;
    
        decomp_load_block(targ_block, slices, slice_len_bytes, offset, i);

        #pragma GCC unroll 8 
        for (size_t j = 0; j < 64; j++)
        {
            // While not all bits unset

            while (targ_block[j])
            {
                ctrl = __builtin_ctzll(targ_block[j]);

                // Rowsum to unset bit
                tableau_rowsum_offset(
                    wid->tableau,
                    ctrl + offset,
                    i + j,
                    offset); 
                
                targ_block[j] = GET_CHUNK(slices, slice_len_bytes, offset, i + j); 
            }
        }
    }

    return;
}


static inline
void decomp_col_elim_lower(
    widget_t* wid,
    void* slices,
    const size_t slice_len_bytes,
    const size_t offset
    )
{
    for (size_t i = 0; i < offset; i += 64) 
    {
        uint64_t targ_block[64];
        uint64_t ctrl;
        
        decomp_load_block(targ_block, slices, slice_len_bytes, offset, i);

        #pragma GCC unroll 8 
        for (size_t j = 0; j < 64; j++)
        {
            // While not all bits unset
            // TODO: merge this line with the ctzll call, odd infinite loop earlier
            while (targ_block[j])
            {
                ctrl = __builtin_ctzll(targ_block[j]);

                // Rowsum to unset bit
                tableau_rowsum_offset(
                    wid->tableau,
                    ctrl + offset,
                    i + j,
                    offset); 
                
                targ_block[j] = GET_CHUNK(slices, slice_len_bytes, offset, i + j); 
            }
        }
    }

    return;
}

void simd_tableau_elim(widget_t* wid)
{

    tableau_t* tab = wid->tableau;
    const size_t tableau_stride = TABLEAU_STRIDE(tab);
    const size_t slice_len_bytes = wid->tableau->slice_len; 

    // All slices will be offset from this pointer
    uint8_t* slices = (void*)(tab->slices_x[0]);
    uint8_t* slices_z = (void*)(tab->slices_z[0]);
    
    uint64_t ctrl_block[64] = {0};
    
    // Stride through the tableau in chunks of 64 elements
    const size_t end_stride = tab->n_qubits - (tab->n_qubits % 64); 

    // TODO set offset to end at end stride
    for (size_t offset = 0;
         offset < tab->n_qubits;
         offset += BLOCK_STRIDE_BITS)
    {
        // Constant control block
        // Allows for quick in-place inspection of the local impact of an xor 

        __inline_decomp_load_block(ctrl_block, slices, slice_len_bytes, offset, offset);

        size_t start_local = 0;
        size_t start = 0;

        // Cleanup the non-diagonal elements in the block
        // TODO: change to block stride bits
        for (size_t j = 0; j < 64; j++)
        {
            
            // Trigger a decomposition
            if (__builtin_ctzll(ctrl_block[j]) != j)
            {
                DPRINT(DEBUG_3, "Triggering Decomposition: %zu\n", offset + j);
                // Try to eliminate elements in the local column
                decomp_local_elim(
                    wid,
                    offset,
                    start_local,
                    j,
                    ctrl_block); 

                start_local = j - 1;

                // Resolved by local elim
                // TODO: Try to avoid a hard switch here  
                if (__builtin_ctzll(ctrl_block[j]) == j)
                {
                    continue;
                }

                DPRINT(DEBUG_3, "Applying Strategies: %zu\n", offset + j);
                __inline_decomp_load_block(ctrl_block, slices, slice_len_bytes, offset, offset);

                DPRINT(DEBUG_3, "Strategy 1: Local Search\n");

                size_t prog = decomp_local_search(
                    wid,
                    offset,
                    j, 
                    ctrl_block
                );

                if (prog < 64)  // Found row, perform swap  
                {
                    continue;
                }

                // Local search failed, try non-local search 
                // TODO: remove reload
                __inline_decomp_load_block(ctrl_block, slices, slice_len_bytes, offset, offset);

                DPRINT(DEBUG_3, "Strategy 2: Non-Local Search\n");
                prog = decomp_non_local_search_and_elim(wid, slices, slice_len_bytes, offset, j);
                if (SENTINEL != prog)  // Found row, perform swap  
                {
                    simd_swap(
                        ((void*)(tab->slices_x[0])) + tab->slice_len * (prog), 
                        ((void*)(tab->slices_z[0])) + tab->slice_len * (prog),
                        ((void*)(tab->slices_x[0])) + tab->slice_len * (offset + j),
                        ((void*)(tab->slices_z[0])) + tab->slice_len * (offset + j),
                        tab->slice_len
                        );

                    ctrl_block[j] = GET_CHUNK(
                            slices,
                            slice_len_bytes,
                            offset,
                            offset + j); 

                        DPRINT(DEBUG_3, "Strategy 2 Succeeded\n");

                    continue;
                }

                // Check if a bare Hadamard will work
                DPRINT(DEBUG_3, "Strategy 3: Hadamard\n");

                if (1 == __inline_slice_get_bit(tab->slices_z[offset + j], offset + j))
                {
                    // TODO: This operation is quite slow
                    simd_tableau_transverse_hadamard(tab, offset + j);
                    clifford_queue_local_clifford_right(wid->queue, _H_, offset + j);
                
                    // Hadamard requires a block reload 
                    __inline_decomp_load_block(ctrl_block, slices, slice_len_bytes, offset, offset);

                    decomp_local_elim(
                        wid,
                        offset,
                        j - 1,
                        j,
                        ctrl_block); 
                    
                    continue;
                }


                // Try searching the Z tableau
                DPRINT(DEBUG_3, "Strategy 4: Hadamard and swap\n");
                prog = decomp_z_search(wid, slices_z, slice_len_bytes, offset, j);
                if (SENTINEL != prog)
                {
                    tableau_idx_swap_transverse(tab, prog, offset + j);
                    simd_tableau_transverse_hadamard(tab, offset + j);

                    // Hadamard requires a block reload
                    __inline_decomp_load_block(ctrl_block, slices, slice_len_bytes, offset, offset);

                    clifford_queue_local_clifford_right(wid->queue, _H_, offset + j);

                    continue;
                }
                else
                {
                    DPRINT(DEBUG_3, "FAILED\n");
                    assert(0);
                }

            }
        }

        DPRINT(DEBUG_3, "Local Block Diagonally decomposable\n");

        // TODO This should be able to start at start_local
        decomp_local_elim(
            wid,
            offset,
            start_local,
            64,
            ctrl_block); 

        decomp_col_elim_upper(wid, slices, slice_len_bytes, offset);
        decomp_col_elim_lower(wid, slices, slice_len_bytes, offset);
    }


    // TODO: Debug info
    for (size_t i = 0; i < tab->n_qubits; i += 64)
    {
        for (size_t j = 0; j < tab->n_qubits; j += 64)
        {
            decomp_load_block(ctrl_block, slices, slice_len_bytes, i, j);
        }
    } 

    return;
}

/*
 *
 */
void zero_z_diagonal(widget_t* wid)
{
    // Phase operation to set Z diagonal to zero
    // Loop acts on separate cache line entries for each element
    for (size_t i = 0; i < wid->tableau->n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
        {
            DPRINT(DEBUG_3, "Applying S to %lu\n", i);
            // Applies the Sdag gate
            // X and Zbit is always 1
            // Operation is r ^= x~z; z ^= x
            // x is 1 via prior diagonalisation, z is 1 by switch invariant
            // Operation is:
            // r ^= 0 -> r
            // z = x.~z = 1.~1 -> 0
            // Hence operation is performed by zeroing the z bit
            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
            // Applies a corresponding S gate
            clifford_queue_local_clifford_right(wid->queue, _S_, i);
        }
    }
}

/*
 * Zeroes the phases in the tableau 
 * :: wid : widget_t* :: Widget object 
 */
void zero_phases(widget_t* wid)
{
    const size_t slice_len = wid->tableau->slice_len;
    // Z to set phases to 0

    // Zero an eight byte block of 64 elements at a time
    for (size_t i = 0; i < slice_len; i += sizeof(uint64_t))
    {
        uint64_t offset = i * 8;
        uint64_t* block = (void*)(wid->tableau->phases) + i;
         
        while (*block)
        {
            uint64_t targ = __builtin_ctzll(*block);
            *block ^= 1ull << targ; 

            // Action of Z gate
            // r ^= x
            // As X is diagonal, this only acts on one bit
            DPRINT(DEBUG_3, "Applying Z to %lu\n", i);
            clifford_queue_local_clifford_right(wid->queue, _Z_, offset + targ);
        }
        // Jump offset by 64
    }
}

/*
 * Naive implementation
 */
void naive_zero_phases(widget_t* wid)
{
    // Z to set phases to 0
    for (size_t i = 0; i < wid->tableau->n_qubits; i++)
    {
        // Action of Z gate
        // r ^= x
        // As X is diagonal, this only acts on one bit
        if (__inline_slice_get_bit(wid->tableau->phases, i))
        {
            DPRINT(DEBUG_3, "Applying Z to %lu\n", i);
            clifford_queue_local_clifford_right(wid->queue, _Z_, i);
        }
    }

    // The previous loop zeros the phases, this loop just does it faster
    // Effective action of a Z gate
    for (size_t i = 0; i < wid->tableau->slice_len / 8; i += 1)
    {
        wid->tableau->phases[i] = 0;
    }
}

/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * Acts in place over the tableau
 */
void tableau_remove_zero_X_columns(tableau_t* tab, clifford_queue_t* c_que)
{
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
       if (CTZ_SENTINEL == tableau_ctz(tab->slices_x[i], tab->slice_len))
       {
         DPRINT(DEBUG_3, "Empty X Col, applying H to %lu\n", i);
         tableau_H(tab, i);
         clifford_queue_local_clifford_right(c_que, _H_, i);
       }
    }
    return;
}

/*
 * tableau_X_diag_element
 * Naive implementation
 * Attempts to implement the gaussian decomp as stated in the paper 
 * :: tab : tableau_t* :: Tableau object 
 * :: queue : clifford_queue_t* :: Clifford queue object 
 * :: idx :: const size_t :: Qubit index 
 * Strategy order is 
 *  - X Column search 
 *  - Hadamard   
 *  - Z Column search
 */
void tableau_X_diag_element(tableau_t* tab, clifford_queue_t* queue, const size_t idx)
{

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            tableau_idx_swap_transverse(tab, idx, j);
            return; 
        } 
    } 

    if (1 == __inline_slice_get_bit(tab->slices_z[idx], idx))
    {
        tableau_transverse_hadamard(tab, idx);
        clifford_queue_local_clifford_right(queue, _H_, idx);   
        return;
    }

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_z[j], idx))
        {
            // Backup Strategy
            tableau_idx_swap_transverse(tab, idx, j);

            tableau_transverse_hadamard(tab, idx);
            clifford_queue_local_clifford_right(queue, _H_, idx);   
            return; 
        } 
    }
    
    return;
}


/*
 * Sets a 1 bit on the diagonal element of the X tableau
 * Returns the index of the permuted row if any   
 */
size_t simd_tableau_X_diag_element(tableau_t* tab, clifford_queue_t* queue, const size_t idx)
{
    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Strategy 1: Swapping %lu <-> %lu\n", j, idx);
            tableau_idx_swap_transverse(tab, idx, j);
            return j;
        }
    }

    
    if (1 == __inline_slice_get_bit(tab->slices_z[idx], idx))
    {
        DPRINT(DEBUG_3, "Strategy 2: Applying Hadamard to %lu\n", idx);
        simd_tableau_transverse_hadamard(tab, idx);
        clifford_queue_local_clifford_right(queue, _H_, idx);
        return SENTINEL;
    }

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_z[j], idx))
        {
            // Backup Strategy
            tableau_idx_swap_transverse(tab, idx, j);

            DPRINT(DEBUG_3, "Strategy 3: Applying Hadamard to %lu\n", idx);

            simd_tableau_transverse_hadamard(tab, idx);
            clifford_queue_local_clifford_right(queue, _H_, idx);
            return j;
        }
    }
    
    return SENTINEL;
}


/*
 * Naive method
 */
void tableau_X_diag_col_upper(tableau_t* tab, const size_t idx)
{
    size_t  j;
    for (j = idx + 1; j < tab->n_qubits; j++)
    {
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            tableau_rowsum(tab, idx, j);
        }
    }
    return;
}

/*
 * Naive method
 */
void tableau_X_diag_col_lower(tableau_t* tab, const size_t idx)
{
    size_t j;
    CHUNK_OBJ* slice = tab->slices_x[idx];
    for (j = 0; j < idx; j++)
    {
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            tableau_rowsum(tab, idx, j);
        }
    }
    return;
}


void simd_tableau_X_diag_col_upper(tableau_t* tab, const size_t idx)
{
    const uint32_t stride = TABLEAU_STRIDE(tab);
    // In preparation for taking this to a gather instruction
    // We collect the whole 8 byte chunk
    void* ptr = ((void*)tab->slices_x[idx]) + (idx / 8);


    static const uint32_t integers[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    const uint64_t mask = 1 << (idx % 8);

	// Load 4 copies of the mask into an int32x4
	int32x4_t v_mask = vdupq_n_s32(mask);
	
    size_t j = 0;
    for (size_t j = idx + 1; j < tab->n_qubits - 3; j+=4)
    {
        // Gather 4 chunks
        // (has to be done one-by-one)
        int32x4_t chunks = vmovq_n_s32(0);

        chunks = vld1q_lane_s32(ptr, chunks, 0);
        chunks = vld1q_lane_s32(ptr + stride, chunks, 0);
        chunks = vld1q_lane_s32(ptr + stride * 2, chunks, 0);
        chunks = vld1q_lane_s32(ptr + stride * 3, chunks, 0);

        // Combine with the mask
        chunks = vandq_s32(chunks, v_mask);

        int32_t dst[4];
        vst1q_s32(dst, chunks);

        #pragma GCC unroll 4
        for (size_t chunk = 0; chunk < 4; chunk++)
        {
            if (dst[chunk])
            {
                DPRINT(DEBUG_3, "Slice XOR Upper: %lu %lu\n", idx, j + chunk);
                tableau_rowsum_offset(tab, idx, j + chunk, j);
            }
        }

        ptr += 4 * stride;
    }

    for (; j < tab->n_qubits; j++)
    {
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            tableau_rowsum_offset(tab, idx, j, j);
        }
    }
    return;

    return;
}


void simd_tableau_X_diag_col_lower(tableau_t* tab, const size_t idx)
{
    for (size_t j = 0; j < idx; j++)
    {
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Slice XOR Lower: %lu %lu\n", idx, j);
            tableau_rowsum_offset(tab, idx, j, j);
        }
    }
    return;
}

/*
 * simd_swap
 * TODO: Fix
 * Swaps indicies over both the X and Z slices
 * Also swaps associated phases
 * :: tab : tableau_t* :: Tableau object to swap over
 * :: i :: const size_t :: Index to swap
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau
 */
void simd_swap(
    void* slice_i_x,
    void* slice_i_z,
    void* slice_j_x,
    void* slice_j_z,
    size_t slice_len)
{
    for (size_t step = 0; step < slice_len; step += 16)
    {
        uint8x16_t v_i_x = vld1q_u8(slice_i_x + step);
        uint8x16_t v_j_x = vld1q_u8(slice_j_x + step);

        uint8x16_t v_i_z = vld1q_u8(slice_i_z + step);
        uint8x16_t v_j_z = vld1q_u8(slice_j_z + step);

        vst1q_u8(slice_i_x + step, v_j_x);
        vst1q_u8(slice_j_x + step, v_i_x);

        vst1q_u8(slice_i_z + step, v_j_z);
        vst1q_u8(slice_j_z + step, v_i_z);
    }

    return;
}

/*
 * tableau_idx_swap_transverse 
 * Swaps indicies over both the X and Z slices  
 * :: tab : tableau_t* :: Tableau object to swap over 
 * :: i :: const size_t :: Index to swap 
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau 
 */
void naive_tableau_idx_swap_transverse(tableau_t* tab, const size_t i, const size_t j)
{
    uint64_t* slice_x_i = tab->slices_x[i];
    uint64_t* slice_x_j = tab->slices_x[j];
    uint64_t* slice_z_i = tab->slices_z[i];
    uint64_t* slice_z_j = tab->slices_z[j];

    assert(i != j);

    // TODO stride this
    // Also TODO this should be simd
    for (size_t idx = 0; idx < tab->slice_len / sizeof(uint64_t); idx++)
    {
        uint64_t tmp =  slice_x_i[idx];
        slice_x_i[idx] = slice_x_j[idx];
        slice_x_j[idx] = tmp; 

        tmp =  slice_z_i[idx];
        slice_z_i[idx] = slice_z_j[idx];
        slice_z_j[idx] = tmp; 
    }

    return;
}
