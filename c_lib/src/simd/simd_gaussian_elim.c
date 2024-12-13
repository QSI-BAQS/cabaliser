#include "simd_gaussian_elimination.h"


/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * Acts in place over the tableau
 */
void tableau_remove_zero_X_columns(tableau_t* tab, clifford_queue_t* c_que)
{
    // Can't parallelise as the H operation is already parallel
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
 * tableau_Z_block_diagonal
 * Ensures that the Z block of the tableau is diagonal
 * Operation unused as it only applies to the non-transposed matrix
 * :: tab : tableau_t* :: Tableau to act on
 *
 */
void tableau_Z_zero_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    for (size_t i = 0; i < tab->n_qubits; i++)
    { 
        if (__inline_slice_get_bit(tab->slices_z[i], i))  
        {
            DPRINT(DEBUG_3, "Zero Diagonal: Phase on %lu ", i);
            tableau_R(tab, i);
            clifford_queue_local_clifford_right(c_que, i, _S_);   
        }
    }
    return;
}

/*
 *
 *
 */
void tableau_X_diag_element(tableau_t* tab, clifford_queue_t* queue, const size_t idx)
{

    for (size_t j = idx + 1; j < tab->n_qubits; j++)
    {
        // Swap stabilisers
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Strategy 1: Swapping %lu <-> %lu\n", j, idx);
            simd_tableau_idx_swap_transverse(tab, idx, j);
            return; 
        } 
    } 

    if (1 == __inline_slice_get_bit(tab->slices_z[idx], idx))
    {
        DPRINT(DEBUG_3, "Strategy 2: Applying Hadamard to %lu\n", idx);
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
            simd_tableau_idx_swap_transverse(tab, idx, j);

            DPRINT(DEBUG_3, "Strategy 3: Applying Hadamard to %lu\n", idx);

            tableau_transverse_hadamard(tab, idx);
            clifford_queue_local_clifford_right(queue, _H_, idx);   
            return; 
        } 
    }
    DPRINT(DEBUG_3, "Applying Hadamard to %lu\n", idx);
    return;
}

void tableau_X_diag_col_upper(tableau_t* tab, const size_t idx)
{
    const uint64_t stride = TABLEAU_STRIDE(tab);
    // In preparation for taking this to a gather instruction
    // We collect the whole 8 byte chunk 
    void* ptr = ((void*)tab->slices_x[idx]) + (idx / 8); 
    const uint64_t mask = 1 << (idx % 8);

    for (size_t j = idx + 1; j < tab->n_qubits; j++) 
    {
        ptr += stride;

        uint64_t chunk = mask & *((uint8_t*)ptr); 
        if (chunk)  
        {
            DPRINT(DEBUG_3, "Slice XOR Upper: %lu %lu\n", idx, j);
            tableau_slice_xor(tab, idx, j);
        }
    }
    return;
}


void tableau_X_diag_col_lower(tableau_t* tab, const size_t idx)
{
    for (size_t j = 0; j < idx; j++) 
    {
        if (1 == __inline_slice_get_bit(tab->slices_x[j], idx))
        {
            DPRINT(DEBUG_3, "Slice XOR Lower: %lu %lu\n", idx, j);
            tableau_slice_xor(tab, idx, j);
        }
    }
    return;
}

/*
 * tableau_X_upper_right_triangular
 * Makes the X block upper right triangular 
 * :: tab : tableau_t* :: Tableau to operate on    
 */
void tableau_X_diagonal(tableau_t* tab, clifford_queue_t* c_que)
{
    tableau_transpose(tab);

    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        // X(i, i) != 1 
        if (0 == __inline_slice_get_bit(tab->slices_x[i], i))
        {  
            tableau_X_diag_element(tab, c_que, i);
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
        __m256i v_i_x = _mm256_load_si256(ptr_i_x += ROWSUM_STRIDE);  
        __m256i v_j_x = _mm256_load_si256(ptr_j_x += ROWSUM_STRIDE);  

        __m256i v_i_z = _mm256_load_si256(ptr_i_z +=  ROWSUM_STRIDE);  
        __m256i v_j_z = _mm256_load_si256(ptr_j_z +=  ROWSUM_STRIDE);  
      
        _mm256_store_si256(ptr_i_x, v_j_x);  
        _mm256_store_si256(ptr_j_x, v_i_x);  

        _mm256_store_si256(ptr_i_z, v_j_z);  
        _mm256_store_si256(ptr_j_z, v_i_z);  
    }

    uint8_t phase_i = slice_get_bit(tab->phases, i); 
    uint8_t phase_j = slice_get_bit(tab->phases, j); 
    slice_set_bit(tab->phases, i, phase_j); 
    slice_set_bit(tab->phases, j, phase_i); 

    return;
}
