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



void debug_dense_print(tableau_t* tab)
{
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j += sizeof(uint64_t))
        {
            printf("%lu ", *(uint64_t*)(((void*)tab->slices_x[i]) + j));
        }
        printf(" | ");
        for (size_t j = 0; j < tab->slice_len; j += sizeof(uint64_t))
        {
            printf("%lu ", *(uint64_t*)(((void*)tab->slices_z[i]) + j));
        }
        printf("\n");
    } 

}

static inline
void zero_z_diagonal(widget_t* wid);

static inline
void zero_phases(widget_t* wid);

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


/*
 * simd_widget_decompose
 * Decomposes the stabiliser tableau into a graph state plus local Cliffords
 * :: wid : widget_t* :: Widget to decompose
 * Acts in place on the tableau
 */
void simd_widget_decompose(widget_t* wid)
{
    tableau_remove_zero_X_columns(
        wid->tableau,
        wid->queue
    );

    tableau_transpose(wid->tableau);

    simd_tableau_elim(wid);
    //tableau_elim_upper(wid);
    //tableau_elim_lower(wid);

    //zero_z_diagonal(wid);

    //zero_phases(wid);

    return;
}


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
              
//               *(uint64_t*)(
//                slices  
//                + slice_len * (col_offset + j) 
//                + row_offset / 8
//            );
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
 * TODO: Comment
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
    uint64_t mask = ((1ull << end) - 1ull);

    // Clean up to the target 
    for (size_t i = 0; i < end; i++)
    {
        while ((63 - __builtin_clzll(mask & ctrl_block[i])) > i)
        {

            ctrl = 63 - __builtin_clzll(mask & ctrl_block[i]);

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
 * TODO
 *
 */
static inline
void decomp_local_elim(
        widget_t* wid,
        const size_t offset,
        const size_t start,
        const size_t end,
        uint64_t ctrl_block[64])

{
    decomp_local_elim_upper(wid, offset, start, end, ctrl_block);
    decomp_local_elim_lower(wid, offset, start, end, ctrl_block);

}


/*
 * Performs a local search
 * Attempts to find a valid index on the control block
 * Strategy 1
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
        printf("Search %zu\n", i);
        if (mask & ctrl_block[i]) 
        {
            uint64_t tmp = ctrl_block[idx]; 
            ctrl_block[idx] = ctrl_block[i]; 
            ctrl_block[i] = tmp; 

            tableau_idx_swap_transverse(
                wid->tableau,
                idx + offset,
                i + offset);
            printf("Search Success: %zu", i);
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

    debug_print_block(targ_block);
    printf("\tSearching local\n");
    // Search local block
    for (size_t j = index + 1; j < 64; j++)
    {
        printf("\t%zu\t%zu\t%u\n", j, mask, !!(targ_block[j] & mask));
        if (targ_block[j] & mask)
        {
            printf("\tFound at %zu\n", j);
            return offset + j; 
        }
    } 

    // Search non-local blocks
    printf("\tSearching non-local\n");
    for (size_t i = offset + 64; i < slice_len_bytes; i += 64) 
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
    const size_t block_end = (wid->tableau->n_qubits / 64) * 64;
    const size_t block_tail = 64 - (wid->tableau->n_qubits % 64);

    // Triggers if block is very small
    if (__builtin_expect(block_end == 0, 0))
    {
        return;
    }
    for (size_t i = offset + 64; i < wid->tableau->n_qubits; i += 64) 
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

    // TODO: Tail

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

    // TODO: Tail

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
                //tableau_print(tab);
                DPRINT(DEBUG_3, "Strategy 1: Local Search\n");

                size_t prog = decomp_local_search(
                    wid,
                    offset,
                    j, 
                    ctrl_block
                );
                printf("Finished local search: %zu \n", prog);

                if (SENTINEL != prog)  // Found row, perform swap  
                {
                    continue;
                }

                // Local search failed, try non-local search 
                DPRINT(DEBUG_3, "Strategy 2: Non-Local Search");
                prog = decomp_non_local_search_and_elim(wid, slices, slice_len_bytes, offset, j);
                if (SENTINEL != prog)  // Found row, perform swap  
                {
                    tableau_idx_swap_transverse(
                        wid->tableau,
                        prog,
                        j + offset);

                    ctrl_block[j] = GET_CHUNK(
                            slices,
                            slice_len_bytes,
                            offset,
                            offset + j); 

                    continue;
                }

                // Check if a bare Hadamard will work
                DPRINT(DEBUG_3, "Strategy 3: Hadamard\n");

                if (1 == __inline_slice_get_bit(tab->slices_z[offset + j], offset + j))
                {
                     debug_print_block(ctrl_block);
                   
                    // TODO: This operation is quite slow
                    tableau_transverse_hadamard(tab, offset + j);
                    clifford_queue_local_clifford_right(wid->queue, _H_, offset + j);
                
                    // Trigger a row reload after the operation
                    ctrl_block[j] = GET_CHUNK(
                            slices,
                            slice_len_bytes,
                            offset,
                            offset + j); 

                    tableau_print(tab);
                    debug_print_block(ctrl_block);

                    decomp_local_elim(
                        wid,
                        offset,
                        j - 1,
                        j,
                        ctrl_block); 
                    
                    debug_print_block(ctrl_block);

                    continue;
                }


                // Try searching the Z tableau
                DPRINT(DEBUG_3, "Strategy 4: Hadamard and swap\n");
                prog = decomp_z_search(wid, slices_z, slice_len_bytes, offset, j);
                if (SENTINEL != prog)
                {
                    printf("Swapping\n");

                    tableau_idx_swap_transverse(tab, offset + prog, offset + j);
                    tableau_transverse_hadamard(tab, offset + j);
                    clifford_queue_local_clifford_right(wid->queue, _H_, offset + j);

                    ctrl_block[j] = GET_CHUNK(
                            slices,
                            slice_len_bytes,
                            offset,
                            offset + j); 

                    printf("Swapped\n");
                }
                else
                {
                    DPRINT(DEBUG_3, "FAILED\n");
                    tableau_print(tab);
                    assert(0);
                }


            }
        }
        // This should be able to start at start_local
        decomp_local_elim(
            wid,
            offset,
            0,
            64,
            ctrl_block); 


        // TODO: Uncomment
        //decomp_col_elim_upper(wid, slices, slice_len_bytes, offset);
        //decomp_col_elim_lower(wid, slices, slice_len_bytes, offset);


    }


    // TODO: Debug info
    for (size_t i = 0; i < tab->n_qubits; i += 64)
    {
        for (size_t j = 0; j < tab->n_qubits; j += 64)
        {
            decomp_load_block(ctrl_block, slices, slice_len_bytes, i, j);
        }
    } 

    printf("Finished\n");
    tableau_print(tab);
    return;
}


void simd_tableau_elim_lower(widget_t* wid)
{

    const size_t tableau_stride = TABLEAU_STRIDE(
                                    wid->tableau);

    // All slices will be offset from this pointer
    uint8_t* slices = (void*)wid->tableau->slices_x[0];

    uint64_t ctrl_block[BLOCK_STRIDE_BITS] = {0};
    
    // TODO: Pipeline this bit
    printf("OPEN LOOP");
    for (size_t i = 0; i < 64; i++)  
    {
        const size_t offset = (i / 64) * 64; 
        const size_t index = i % 64;

        // Decompose local block
        uint64_t diag = *(uint64_t*)(slices + tableau_stride * i + 64 * offset); 
        uint64_t ctrl = 0;
        printf("%zu\n", ctrl);

        while ((ctrl = __builtin_clzll(diag)) > index)
        { 
            printf("%zu\n", ctrl);
            tableau_rowsum_offset(wid->tableau, ctrl + offset, diag + offset, offset);
        }
    }
}

/*
 *
 */ 
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
 *
 */
static inline
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
 *
 */
static inline
void zero_phases(widget_t* wid)
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
    for (size_t i = 0; i < wid->tableau->slice_len; i++)
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
            simd_tableau_idx_swap_transverse(tab, idx, j);
            return j;
        }
    }

    
    if (1 == __inline_slice_get_bit(tab->slices_z[idx], idx))
    {
        DPRINT(DEBUG_3, "Strategy 2: Applying Hadamard to %lu\n", idx);
        tableau_transverse_hadamard(tab, idx);
        clifford_queue_local_clifford_right(queue, _H_, idx);
        return SENTINEL;
    }

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_z[j], idx))
        {
            // Backup Strategy
            simd_tableau_idx_swap_transverse(tab, idx, j);

            DPRINT(DEBUG_3, "Strategy 3: Applying Hadamard to %lu\n", idx);

            tableau_transverse_hadamard(tab, idx);
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


#define SCALE 4
void simd_tableau_X_diag_col_upper(tableau_t* tab, const size_t idx)
{
    const uint32_t stride = TABLEAU_STRIDE(tab);
    // In preparation for taking this to a gather instruction
    // We collect the whole 8 byte chunk
    void* ptr = ((void*)tab->slices_x[idx]) + (idx / 8);


    static const uint32_t integers[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    const uint64_t mask = 1 << (idx % 8);

    // 8 Copies of the mask
    __m256i v_mask = _mm256_broadcastd_epi32(_mm_loadu_si32(&mask));
    __m256i pointer_offset = _mm256_mul_epi32(
                        _mm256_lddqu_si256((void*)integers),
                        _mm256_broadcastd_epi32(_mm_loadu_si32(&stride))    
                    );
 
    size_t j = 0;
    for (size_t j = idx + 1; j < tab->n_qubits - 7; j+=8)
    {
        
        // Gathers 8 chunks
        __m256i chunks = _mm256_and_si256(v_mask, _mm256_i32gather_epi32(ptr, pointer_offset, SCALE));
        uint32_t dst[8];
        _mm256_storeu_si256((void*)dst, chunks);

        #pragma GCC unroll 8
        for (size_t chunk = 0; chunk < 8; chunk++)
        {
            if (dst[chunk])
            {
                DPRINT(DEBUG_3, "Slice XOR Upper: %lu %lu\n", idx, j + chunk);
                tableau_rowsum_offset(tab, idx, j + chunk, j);
            }
        }

        ptr += 8 * stride;
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
 * simd_tableau_idx_swap_transverse
 * Swaps indicies over both the X and Z slices
 * Also swaps associated phases
 * :: tab : tableau_t* :: Tableau object to swap over
 * :: i :: const size_t :: Index to swap
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau
 */
void simd_tableau_idx_swap_transverse(tableau_t* restrict tab, const size_t i, const size_t j)
{
    void* restrict ptr_i_x = tab->slices_x[i];
    void* restrict ptr_j_x = tab->slices_x[j];
    void* restrict ptr_i_z = tab->slices_z[i];
    void* restrict ptr_j_z = tab->slices_z[j];

    for (size_t step = 0; step < tab->slice_len * sizeof(CHUNK_OBJ); step += ROWSUM_STRIDE)
    {
        __m256i v_i_x = _mm256_load_si256(ptr_i_x + step);
        __m256i v_j_x = _mm256_load_si256(ptr_j_x + step);

        __m256i v_i_z = _mm256_load_si256(ptr_i_z + step);
        __m256i v_j_z = _mm256_load_si256(ptr_j_z + step);

        _mm256_store_si256(ptr_i_x + step, v_j_x);
        _mm256_store_si256(ptr_j_x + step, v_i_x);

        _mm256_store_si256(ptr_i_z + step, v_j_z);
        _mm256_store_si256(ptr_j_z + step, v_i_z);
    }

//    uint8_t phase_i = slice_get_bit(tab->phases, i);
//    uint8_t phase_j = slice_get_bit(tab->phases, j);
//    slice_set_bit(tab->phases, i, phase_j);
//    slice_set_bit(tab->phases, j, phase_i);

    return;
}
