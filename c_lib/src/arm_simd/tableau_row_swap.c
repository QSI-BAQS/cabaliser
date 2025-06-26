#include "rowswap.h"

void simd_row_swap(
    const size_t n_bytes,
    void* restrict ctrl_x, 
    void* restrict ctrl_z, 
    void* restrict targ_x, 
    void* restrict targ_z 
)
{
    for (size_t i = 0; i < n_bytes; i += ROWSUM_STRIDE)
    {
        __m256i v_targ_x = _mm256_loadu_si256(
            targ_x + i 
        );

        __m256i v_ctrl_x = _mm256_loadu_si256(
            ctrl_x + i 
        );

        __m256i v_targ_z = _mm256_loadu_si256(
            targ_z + i 
        );

        __m256i v_ctrl_z = _mm256_loadu_si256(
            ctrl_z + i 
        );

        _mm256_storeu_si256(targ_x + i, v_ctrl_x);
        _mm256_storeu_si256(targ_z + i, v_ctrl_z);
        _mm256_storeu_si256(ctrl_x + i, v_targ_x);
        _mm256_storeu_si256(ctrl_z + i, v_targ_z);

    }
}
