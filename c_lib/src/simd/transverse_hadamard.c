#include "transverse_hadamard.h"

/*
 * simd_tableau_transverse_hadamard
 * Applies a hadamard when transposed 
 * :: tab : tableau_t*  :: Tableau object
 * :: c_que :  clifford_queue_t* :: Clifford queue 
 * :: i : const size_t :: Index to target 
 *
 */
void simd_tableau_transverse_hadamard(tableau_t const* tab, const size_t targ)
{ 
    const size_t stride = 64;
    uint8_t bit_x[64] = {0};
    uint8_t bit_z[64] = {0};
    uint64_t* bit_phase;
  
    // Number of bytes between transverseally adjacent bytes  
    const size_t row_bytes = tab->slice_len;
    const size_t offset = targ / 64 * 8; // Aligned offset load  

    // TODO Vectorise this
    // Use the strided load to do this faster
    // Unfortunately scatter operations are avx512 :(

    // Gathers 8 values
    //__m256i gathered = _mm256_i32gather_epi64(
    //    base_address
    //    __m128i vindex,
    //    1
    //); 

    // TODO SIMD this by chunks
    // Otherwise we're trying to pipeline this
    for (size_t i = 0; i < tab->n_qubits; i += stride)
    {
        bit_phase = (uint64_t*)(tab->phases) + i / stride; 
        //#pragma GCC unroll 64 
        for (size_t j = 0; j < stride; j++)
        {
            bit_z[j] = __inline_slice_get_bit(tab->slices_z[i + j], targ); 
            bit_x[j] = __inline_slice_get_bit(tab->slices_x[i + j], targ); 
        }

        //#pragma GCC unroll 64 
        for (size_t j = 0; j < stride; j++)
        {
            *bit_phase ^= ((uint64_t)bit_z[j] & (uint64_t)bit_x[j]) << j;
        }
       
        //#pragma GCC unroll 64 
        for (size_t j = 0; j < stride; j++)
        {
            __inline_slice_set_bit(tab->slices_z[i + j], targ, bit_x[j]); 
            __inline_slice_set_bit(tab->slices_x[i + j], targ, bit_z[j]); 
        }
    }
    return;
}
