#include "simd_gaussian_elimination.h"

#define SENTINEL (-1ll) 

void debug_print_block(uint64_t block[64])
{
    for (size_t i = 0; i < 8; i++)
    {
        printf("-");
    }
    printf("\n");

    for (size_t i = 0; i < 8; i++)
    {
        printf("|");
        for (size_t j = 0; j < 8; j++)
        {
           printf("%d", !!(block[j] & (1ull << i))); 
        }
        printf("|\n");
    }
    for (size_t i = 0; i < 8; i++)
    {
        printf("-");
    }
    printf("\n");
}


static inline
void zero_z_diagonal(widget_t* wid);

static inline
void zero_phases(widget_t* wid);

/*
 * simd_widget_decompose
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
    //tableau_print(wid->tableau);
    //printf("#####\n");
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
    //tableau_print(wid->tableau);
}



#define BLOCK_STRIDE_BITS 64
#define BLOCK_STRIDE_BYTES 8
void decomp_load_ctrl_block(
    uint64_t ctrl_block[64],
    void* slices, 
    size_t stride,
    size_t offset
)
{
        #pragma GCC unroll 64 
        for (size_t j = 0; j < BLOCK_STRIDE_BITS; j++)
        {
           ctrl_block[j] = *(uint64_t*)(
                slices  
                + (offset + j) * stride 
                + (offset / BLOCK_STRIDE_BYTES)
            );
        }
}


void decomp_local_elim(
        widget_t* wid,
        const size_t offset,
        const size_t start,
        const size_t end,
        uint64_t ctrl_block[64])
{

    size_t elim_tracer = 0;
    size_t ctrl = 0;

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
    // TODO this should be 64
    for (size_t i = end; i < 8; i++)
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

size_t decomp_local_X(
        widget_t* wid,
        const size_t offset,
        const size_t idx,
        uint64_t ctrl_block[64])
{
    const uint64_t mask = (1ull << idx);
    for (size_t i = idx + 1; i < 8; i++)
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

void simd_tableau_elim_upper(widget_t* wid)
{
    const size_t tableau_stride = TABLEAU_STRIDE(
                                    wid->tableau);

    // All slices will be offset from this pointer
    uint8_t* slices = (void*)wid->tableau->slices_x[0];

    for (size_t i = 0; i < 8; i++)
    {
        printf("%p, ", slices + i * tableau_stride);
    }
    printf("\n");
  
    for (size_t i = 0; i < 8; i++)
    {
        printf("%p, ", wid->tableau->slices_x[i]);
    }
    printf("\n");

    uint64_t ctrl_block[BLOCK_STRIDE_BITS] = {0};
 
    // Stride through the tableau in chunks of 64 elements
    for (size_t offset = 0;
         offset < wid->n_qubits;
         offset += BLOCK_STRIDE_BITS)
    {
        // Constant control block
        // Allows for quick in-place inspection of the local impact of an xor 
        decomp_load_ctrl_block(ctrl_block, slices, tableau_stride, offset);

        size_t start_local = 0;
        size_t start = 0;

        // Cleanup the non-diagonal elements in the block
        // TODO: change to block stride bits
        for (size_t j = 0; j < wid->n_qubits; j++)
        {
            printf("%lu\n", ctrl_block[j] & (1lu << j));
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
              
                // Strategy #1 
                // Perform a local swap
                size_t prog = decomp_local_X(
                    wid,
                    offset,
                    j, 
                    ctrl_block
                );

                // Local swap failed  
                if (SENTINEL == prog)
                {

                    // Strategy #2 - Try a Hadamard
                    printf("Attempting Hadamard\n");
                    if (1 == __inline_slice_get_bit(wid->tableau->slices_z[offset + j], offset + j))
                    {
                        DPRINT(DEBUG_3, "Strategy 2: Applying Hadamard to %lu\n", idx);
                        tableau_transverse_hadamard(wid->tableau, offset + j);
                        clifford_queue_local_clifford_right(wid->queue, _H_, offset + j);

                        ctrl_block[j] = *(uint64_t*)(
                            slices  
                            + tableau_stride * j
                            + (offset / 64) * 64 
                        );
                    }


                    printf("Local Swap Failed\n");
                }

            }

            debug_print_block(ctrl_block);
            printf("%lu %u\n", j, BLOCK_STRIDE_BITS);
        }
    }


    for (size_t i = 0; i < 8; i++)
    {
        printf("%p, ", slices + i * tableau_stride);
    }
    printf("\n");
  
    for (size_t i = 0; i < 8; i++)
    {
        printf("%p, ", wid->tableau->slices_x[i]);
    }
    printf("\n");


    debug_print_block(ctrl_block);

    decomp_load_ctrl_block(ctrl_block, slices, tableau_stride, 0);
    debug_print_block(ctrl_block);


    printf("END\n");
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
    for (size_t i = 0; i < wid->n_qubits; i++)  
    {
        const size_t offset = (i / 64) * 64; 
        const size_t index = i % 64;

        // Decompose local block
        uint64_t diag = *(uint64_t*)(slices + tableau_stride * i + 64 * offset); 
        uint64_t ctrl = 0;
        while ((ctrl = __builtin_clzll(diag)) > index)
        { 
             tableau_rowsum_offset(wid->tableau, ctrl + offset, diag + offset, offset);
        }
    }
}



 
            // From the previous loop, the ctzll call here should be upper bounded by j
//            for (size_t k = j + 1; k < BLOCK_STRIDE_BITS; k++)
//            {
//                if (ctrl_block[k] & (1ull << j))  
//                {
//                    tableau_slice_xor(wid->tableau, i + ctrl, i + j);
//                    ctrl_block[k] ^= ctrl_block[j];
//                }
//            }
        //debug_print_block(ctrl_block);


       // // Target block initial allocation
       // uint64_t target_block[BLOCK_STRIDE_BITS] = {0};
       // // Eliminate diagonal elements in subsequent blocks
       // for (size_t j = i + BLOCK_STRIDE_BITS; j < wid->n_qubits; j += BLOCK_STRIDE_BITS)
       // {
       //     // We can pipeline this section
       //     // Triggering a copy on this block also keeps it local if an xor flushes cache
       //     #pragma GCC unroll 64 
       //     for (size_t j = 0; j < BLOCK_STRIDE_BITS; j++)
       //     {
       //        target_block[j] = *(uint64_t*)(slices + (i + j) * tableau_stride + (i / BLOCK_STRIDE_BYTES));
       //     }

       //     for (size_t k = 0; j < BLOCK_STRIDE_BITS; k++)
       //     {
       //         // Converts up to n branch predictor misses into 1 miss
       //         while (target_block[k])
       //         {
       //             size_t targ = __builtin_ctzll(target_block[k]);
       //             //printf("Targ: %lu\n", targ);
       //             target_block[k] ^= ctrl_block[targ]; 
       //             tableau_slice_xor(wid->tableau, i + targ, j + k);
       //         }
       //     } 
       // }

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



// Naive implementation
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
    //assert(0); // Could not place a 1 in the X diagonal
    
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
    
    // Failed
    //assert(0);
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

    uint8_t phase_i = slice_get_bit(tab->phases, i);
    uint8_t phase_j = slice_get_bit(tab->phases, j);
    slice_set_bit(tab->phases, i, phase_j);
    slice_set_bit(tab->phases, j, phase_i);

    return;
}
