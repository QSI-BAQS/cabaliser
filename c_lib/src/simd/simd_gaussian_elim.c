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

    tableau_elim_upper(wid);

    tableau_elim_lower(wid);

    zero_z_diagonal(wid);

    zero_phases(wid);

    return;
}


void tableau_elim_upper(widget_t* wid)
{
    for (size_t i = 0; i < wid->n_qubits; i++)
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
static inline
void __inline_decomp_load_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
)
{
        printf("Block Loaded From: %zu %zu, slice_len: %zu \n", row_offset, col_offset, slice_len);
        #pragma GCC unroll 64 
        for (size_t j = 0; j < BLOCK_STRIDE_BITS; j++)
        {
           block[j] = *(uint64_t*)(
                slices  
                + slice_len * (col_offset + j) 
                + row_offset / 8
            );
           printf("%p ", (void*)(
                slices  
                + slice_len * (col_offset + j) 
                + row_offset / 8
            ));
        }
       printf("\n");
}
void decomp_load_block(
    uint64_t block[64],
    void* slices, 
    const size_t slice_len,
    const size_t row_offset,
    const size_t col_offset 
)
{
    printf("Loading \n");
    __inline_decomp_load_block(block, slices, slice_len, row_offset, col_offset);
    debug_print_block(block);
}
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
    printf("Storing\n");
    __inline_decomp_store_block(block, slices, slice_len, row_offset, col_offset);
    debug_print_block(block);
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

    if (__builtin_expect((wid->n_qubits < offset + 64), 0))
    {
        // TODO: Profile whether this branch is better, 
        // or having a dedicated tailing function and unroll
        // the loop is better
        block_end = wid->n_qubits % 64; 
    }

    // Clean up to the target 
    for (size_t i = start; i < end; i++)
    {
        while (
        i > 
        (ctrl = __builtin_ctzll(ctrl_block[i])))
        {
            //printf("Prior: %zu %zu\n", ctrl, i);
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
 */
static inline
void decomp_local_elim_lower(
        widget_t* wid,
        const size_t offset,
        uint64_t ctrl_block[64])
{

    size_t elim_tracer = 0;
    size_t ctrl = 0;

    size_t block_end = 64;
    if (__builtin_expect((wid->n_qubits < offset + 64), 0))
    {
        // TODO: Profile whether this branch is better, or having a dedicated
        // tailing function and unrolling this is better 
        block_end = wid->n_qubits % 64; 
    }

    // Clean up to the target 
    for (size_t i = 0; i < block_end; i++)
    {
        while (
        i < 
        (ctrl = 63 - __builtin_clzll(ctrl_block[i])))
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
        if (mask & ctrl_block[i]) 
        {
            uint64_t tmp = ctrl_block[idx]; 
            ctrl_block[idx] = ctrl_block[i]; 
            ctrl_block[i] = tmp; 

            tableau_idx_swap_transverse(
                wid->tableau,
                idx + offset,
                i + offset);

            return idx; 
        }
    } 
    return SENTINEL;
}

/*
 * Performs a search and progressive elimination as one step 
 * This search is non-local, it begins on the tile subsequent to the diagonal 
 */
static inline
size_t decomp_non_local_search_and_elim(
    widget_t* wid,
    size_t stride,
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

    for (size_t i = offset + stride; i < block_end; i += stride) 
    {
        uint64_t targ_block[64];
        uint64_t ctrl;
        
        // Pipeline load to cache line
        #pragma GCC unroll 64 
        for (size_t j = 0; j < 64; j++)
        {
            targ_block[j] = *(uint64_t*)(
                slices  
                + (i + j) * slice_len_bytes // Row 
                + column // Column
            );
        }

        printf("Column State Initial\n");
        printf("Start: %zu\n", i);
        printf("Target: %zu\n", index);
        printf("Offset: %zu\n", offset);

        debug_print_block(targ_block);

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
                targ_block[j] = *(uint64_t*)(
                    slices  
                    + (i + j) * stride // Row 
                    + column // Column 
                );
            }
            if (__builtin_expect(targ_block[j] & mask, 0))
            {
                printf("Column State Final\n");
                printf("Target: %zu\n", index);
                printf("Row: %zu\n", j);

                debug_print_block(targ_block);

                return i + j; 
            }
        }
    }

    // Could stride this for more pipelining
    for (size_t i = block_end; i < wid->tableau->n_qubits; i+= BLOCK_STRIDE_BITS) 
    {
        uint64_t targ_block[64];
        uint64_t ctrl;
        
        // Pipeline load to cache line
        #pragma GCC unroll 64 
        for (size_t j = 0; j < block_tail; j++)
        {
            targ_block[j] = *(uint64_t*)(
                slices  
                + (i + j) * slice_len_bytes // Row 
                + (offset / 64) // Column
            );
        }

        printf("Column State Initial\n");
        printf("Start: %zu\n", offset);

        debug_print_block(targ_block);

        #pragma GCC unroll 64 
        for (size_t j = 0; j < block_tail; j++)
        {
            // While not all bits unset
            while ((ctrl = __builtin_ctzll(targ_block[j])) < index)
            {
                // Rowsum to unset bit
                // Might be better on the stride detector to do this backwards
                tableau_rowsum_offset(
                    wid->tableau,
                    ctrl + offset,
                    i + j,
                    offset); 
                
                // Reload block
                targ_block[j] = *(uint64_t*)(
                    slices  
                    + (i + j) * slice_len_bytes // Row 
                    + (offset / 64) // Column 
                );
            }
            if (__builtin_expect(targ_block[j] & mask, 0))
            {
                printf("Found at end %zu %zu \n", block_end, j);
                return block_end + j; 
            }
        }
    }

    printf("Not found\n");

    return SENTINEL;
}
    


void simd_tableau_elim_upper(widget_t* wid)
{

    tableau_t* tab = wid->tableau;
    const size_t tableau_stride = TABLEAU_STRIDE(tab);
    const size_t slice_len_bytes = wid->tableau->slice_len; 
    printf("STRIDE %zu\n", tableau_stride);

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
        decomp_load_block(ctrl_block, slices, slice_len_bytes, offset, offset);

        printf("New Block\n");
        debug_print_block(ctrl_block);

        size_t start_local = 0;
        size_t start = 0;

        // Cleanup the non-diagonal elements in the block
        // TODO: change to block stride bits
        for (size_t j = 0; j < 64; j++)
        {
            
            printf("Lookup: %lu %lu %lu %u %u\n",
                wid->n_qubits,
                offset,
                j,
                __builtin_ctzll(ctrl_block[j]),
                __builtin_ctzll(*(tab->slices_x[offset + j] + offset / 64)));

            // Trigger a decomposition
            if (__builtin_ctzll(ctrl_block[j]) != j)
            {
                printf("Block: %lu %lu %u\n", offset, j, __builtin_ctzll(ctrl_block[j]));
                // Try to eliminate elements in the local column
                printf("Attempting elim upper\n");
                decomp_local_elim_upper(
                    wid,
                    offset,
                    start_local,
                    j,
                    ctrl_block); 

                start_local = j - 1;
               
                debug_print_block(ctrl_block);

                printf("Local Search\n "); 
                DPRINT(DEBUG_3, "Strategy 1: Local Search", idx);
                size_t prog = decomp_local_search(
                    wid,
                    offset,
                    j, 
                    ctrl_block
                );

                debug_print_block(ctrl_block);

                // Local search failed  
                if (SENTINEL == prog)
                {

                    printf("Considering Hadamard\n");
                    // Strategy #2 - Try a Hadamard
                    if (1 == __inline_slice_get_bit(tab->slices_z[offset + j], offset + j))
                    {
                        printf("Performing Hadamard\n "); 
                        DPRINT(DEBUG_3, "Strategy 2: Applying Hadamard to %lu\n", idx);
                        // This operation is quite slow
                        tableau_transverse_hadamard(tab, offset + j);
                        clifford_queue_local_clifford_right(wid->queue, _H_, offset + j);

                        ctrl_block[j] = *(uint64_t*)(
                            slices  
                            + tableau_stride * j
                            + (offset / 64) * 64 
                        );
                        debug_print_block(ctrl_block);
                    } else {

                        DPRINT(DEBUG_3, "Strategy 3: Search Column", idx);

                        //debug_print_block(ctrl_block);
                        //printf("%lu %lu %lu", wid->n_qubits, offset, j); 

                        printf("Search Column\n "); 
                        prog = decomp_non_local_search_and_elim(wid, tableau_stride, slices, slice_len_bytes, offset, j);
                        if (SENTINEL == prog)
                        {
                            printf("Hadamard Search");
                            DPRINT(DEBUG_3, "Strategy 4: Hadamard and Search Column", idx);
                        }
                        else
                        {

                            // Update before swap avoids reloading into a
                            // cache miss if the row is large enough to flush
                            // the cache 
                            ctrl_block[j] = *(uint64_t*)(
                                slices  
                                + slice_len_bytes * (offset + prog)
                                + offset 
                            );

                            tableau_idx_swap_transverse(
                                wid->tableau,
                                prog,
                                j + offset);

                            debug_print_block(ctrl_block);

                        }
                    }
                }
                printf("After Elim: %d\n", __builtin_ctzll(ctrl_block[j]));

            }

            printf("%lu %u\n", j, BLOCK_STRIDE_BITS);
        }

        //printf("Starting Lower\n");
        //decomp_local_elim_lower(wid, offset, ctrl_block);
        //printf("Finished Lower\n");
    }


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
    for (size_t i = 0; i < wid->n_qubits; i++)
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
    for (size_t i = 0; i < wid->n_qubits; i++)
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
    for (size_t i = 0; i < wid->n_qubits; i++)
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
