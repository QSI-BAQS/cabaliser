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
        TABLEAU_SIMD_VEC v_targ_x = vld1q_NEON_SUFFIX(
            targ_x + i 
        );

        TABLEAU_SIMD_VEC v_ctrl_x = vld1q_NEON_SUFFIX(
            ctrl_x + i 
        );

        TABLEAU_SIMD_VEC v_targ_z = vld1q_NEON_SUFFIX(
            targ_z + i 
        );

        TABLEAU_SIMD_VEC v_ctrl_z = vld1q_NEON_SUFFIX(
            ctrl_z + i 
        );

        vst1q_NEON_SUFFIX(targ_x + i, v_ctrl_x);
        vst1q_NEON_SUFFIX(targ_z + i, v_ctrl_z);
        vst1q_NEON_SUFFIX(ctrl_x + i, v_targ_x);
        vst1q_NEON_SUFFIX(ctrl_z + i, v_targ_z);

    }
}
