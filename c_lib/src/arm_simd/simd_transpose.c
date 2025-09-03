#include "simd_transpose.h"

/*
 * Interal bit manipulation functions (naive)
 *
 * ARM Neon lacks ext, dup, etc. so either;
 *      1. SVE is required
 *      2. Software versions must be used
 *
 * Naive versions are left for testing & handling arbitrary masks
 */

static inline uint64_t __naive_bext_u64(uint64_t targ, uint64_t mask)
{
    uint64_t res = 0;
    int r_pos = 0;

    #pragma GCC unroll 64
    for (int i = 0; i < 64; i++)
    {
        if (mask & (1ull << i))
        {
            if (targ & (1ull << i))
            {
                res |= 1ull << r_pos;
            }

            r_pos++;
        }
    }

    return res;
}

static inline uint64_t __naive_bdep_u64(uint64_t targ, uint64_t mask)
{
    uint64_t res = 0;
    int t_pos = 0;

    #pragma GCC unroll 64
    for (int i = 0; i < 64; i++)
    {
        if (mask & (1ull << i))
        {
            if (targ & (1ull << t_pos))
            {
                res |= 1ull << i; 
            }

            t_pos++;
        }
    }

    return res;
}

/*
 * Internal bit manipulation functions (per-mask)
 */
// bext for predefined mask 0x0101010101010101ull
static inline uint64_t _bext_u64_mask01(uint64_t targ)
{
    uint64_t res = 0;

    // targ[0] -> res[0]
    res |= (targ & 1ull);

    // targ[8] -> res[1]
    res |= (targ & (1ull << 8)) >> 7;

    // targ[16] -> res[2]
    res |= (targ & (1ull << 16)) >> 14;

    // targ[24] -> res[3]
    res |= (targ & (1ull << 24)) >> 21;

    // targ[32] -> res[4]
    res |= (targ & (1ull << 32)) >> 28;

    // targ[40] -> res[5]
    res |= (targ & (1ull << 40)) >> 35;

    // targ[48] -> res[6]
    res |= (targ & (1ull << 48)) >> 42;

    // targ[56] -> res[7];
    res |= (targ & (1ull << 56)) >> 49;

    return res;
}

// bext for predefined mask 0x0202020202020202ull
static inline uint64_t _bext_u64_mask02(uint64_t targ)
{
    uint64_t res = 0;

    // targ[1] -> res[0]
    res |= (targ & (1ull << 1)) >> 1;

    // targ[9] -> res[1]
    res |= (targ & (1ull << 9)) >> 8;

    // targ[17] -> res[2]
    res |= (targ & (1ull << 17)) >> 15;

    // targ[25] -> res[3]
    res |= (targ & (1ull << 25)) >> 22;

    // targ[33] -> res[4]
    res |= (targ & (1ull << 33)) >> 29;

    // targ[41] -> res[5]
    res |= (targ & (1ull << 41)) >> 36;

    // targ[49] -> res[6]
    res |= (targ & (1ull << 49)) >> 43;

    // targ[57] -> res[7];
    res |= (targ & (1ull << 57)) >> 50;

    return res;
}

// bext for predefined mask 0x0404040404040404ull
static inline uint64_t _bext_u64_mask04(uint64_t targ)
{
    uint64_t res = 0;

    // targ[2] -> res[0]
    res |= (targ & (1ull << 2)) >> 2;

    // targ[10] -> res[1]
    res |= (targ & (1ull << 10)) >> 9;

    // targ[18] -> res[2]
    res |= (targ & (1ull << 18)) >> 16;

    // targ[26] -> res[3]
    res |= (targ & (1ull << 26)) >> 23;

    // targ[34] -> res[4]
    res |= (targ & (1ull << 34)) >> 30;

    // targ[42] -> res[5]
    res |= (targ & (1ull << 42)) >> 37;

    // targ[50] -> res[6]
    res |= (targ & (1ull << 50)) >> 44;

    // targ[58] -> res[7];
    res |= (targ & (1ull << 58)) >> 51;

    return res;
}

// bext for predefined mask 0x0808080808080808ull
static inline uint64_t _bext_u64_mask08(uint64_t targ)
{
    uint64_t res = 0;

    // targ[3] -> res[0]
    res |= (targ & (1ull << 3)) >> 3;

    // targ[11] -> res[1]
    res |= (targ & (1ull << 11)) >> 10;

    // targ[19] -> res[2]
    res |= (targ & (1ull << 19)) >> 17;

    // targ[27] -> res[3]
    res |= (targ & (1ull << 27)) >> 24;

    // targ[35] -> res[4]
    res |= (targ & (1ull << 35)) >> 31;

    // targ[43] -> res[5]
    res |= (targ & (1ull << 43)) >> 38;

    // targ[51] -> res[6]
    res |= (targ & (1ull << 51)) >> 45;

    // targ[59] -> res[7];
    res |= (targ & (1ull << 59)) >> 52;

    return res;
}

// bext for predefined mask 0x1010101010101010ull 
static inline uint64_t _bext_u64_mask10(uint64_t targ)
{
    uint64_t res = 0;

    // targ[4] -> res[0]
    res |= (targ & (1ull << 4)) >> 4;

    // targ[12] -> res[1]
    res |= (targ & (1ull << 12)) >> 11;

    // targ[20] -> res[2]
    res |= (targ & (1ull << 20)) >> 18;

    // targ[28] -> res[3]
    res |= (targ & (1ull << 28)) >> 25;

    // targ[36] -> res[4]
    res |= (targ & (1ull << 36)) >> 32;

    // targ[44] -> res[5]
    res |= (targ & (1ull << 44)) >> 39;

    // targ[52] -> res[6]
    res |= (targ & (1ull << 52)) >> 46;

    // targ[60] -> res[7];
    res |= (targ & (1ull << 60)) >> 53;

    return res;
}

// bext for predefined mask 0x2020202020202020ull 
static inline uint64_t _bext_u64_mask20(uint64_t targ)
{
    uint64_t res = 0;

    // targ[5] -> res[0]
    res |= (targ & (1ull << 5)) >> 5;

    // targ[13] -> res[1]
    res |= (targ & (1ull << 13)) >> 12;

    // targ[21] -> res[2]
    res |= (targ & (1ull << 21)) >> 19;

    // targ[29] -> res[3]
    res |= (targ & (1ull << 29)) >> 26;

    // targ[37] -> res[4]
    res |= (targ & (1ull << 37)) >> 33;

    // targ[45] -> res[5]
    res |= (targ & (1ull << 45)) >> 40;

    // targ[53] -> res[6]
    res |= (targ & (1ull << 53)) >> 47;

    // targ[61] -> res[7];
    res |= (targ & (1ull << 61)) >> 54;

    return res;
}

// bext for predefined mask 0x4040404040404040ull 
static inline uint64_t _bext_u64_mask40(uint64_t targ)
{
    uint64_t res = 0;

    // targ[6] -> res[0]
    res |= (targ & (1ull << 6)) >> 6;

    // targ[14] -> res[1]
    res |= (targ & (1ull << 14)) >> 13;

    // targ[22] -> res[2]
    res |= (targ & (1ull << 22)) >> 20;

    // targ[30] -> res[3]
    res |= (targ & (1ull << 30)) >> 27;

    // targ[38] -> res[4]
    res |= (targ & (1ull << 38)) >> 34;

    // targ[46] -> res[5]
    res |= (targ & (1ull << 46)) >> 41;

    // targ[54] -> res[6]
    res |= (targ & (1ull << 54)) >> 48;

    // targ[62] -> res[7];
    res |= (targ & (1ull << 62)) >> 55;

    return res;
}

// bext for predefined mask 0x8080808080808080ull 
static inline uint64_t _bext_u64_mask80(uint64_t targ)
{
    uint64_t res = 0;

    // targ[7] -> res[0]
    res |= (targ & (1ull << 7)) >> 7;

    // targ[15] -> res[1]
    res |= (targ & (1ull << 15)) >> 14;

    // targ[23] -> res[2]
    res |= (targ & (1ull << 23)) >> 21;

    // targ[31] -> res[3]
    res |= (targ & (1ull << 31)) >> 28;

    // targ[39] -> res[4]
    res |= (targ & (1ull << 39)) >> 35;

    // targ[47] -> res[5]
    res |= (targ & (1ull << 47)) >> 42;

    // targ[55] -> res[6]
    res |= (targ & (1ull << 55)) >> 49;

    // targ[63] -> res[7];
    res |= (targ & (1ull << 63)) >> 56;

    return res;
}

// bdep for predefined mask 0x00000000000000ffull
static inline uint64_t _bdep_u64_maskff(uint64_t targ)
{
    uint64_t res = 0;

    res |= targ & 0xffull;

    return res;
}

// bdep for predefined mask 0x000000000000ff00ull
static inline uint64_t _bdep_u64_maskff00(uint64_t targ)
{
    uint64_t res = 0;

    res |= (targ & 0xffull) << 8;

    return res;
}



// Transposes two blocks
static inline
void __inline_simd_transpose_2x16(uint8_t** restrict src, uint8_t** restrict targ)
{
    // Do an upper and lower pass (half size Neon registers)
    uint16x8_t src_v_lower = vdupq_n_u16(0);

    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[0], src_v_lower, 0);
    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[1], src_v_lower, 1);
    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[2], src_v_lower, 2);
    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[3], src_v_lower, 3);
    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[4], src_v_lower, 4);
    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[5], src_v_lower, 5);
    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[6], src_v_lower, 6);
    src_v_lower = vsetq_lane_u16(*(uint16_t *)src[7], src_v_lower, 7);

    uint16x8_t src_v_upper = vdupq_n_u16(0);

    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[8], src_v_upper, 0);
    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[9], src_v_upper, 1);
    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[10], src_v_upper, 2);
    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[11], src_v_upper, 3);
    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[12], src_v_upper, 4);
    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[13], src_v_upper, 5);
    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[14], src_v_upper, 6);
    src_v_upper = vsetq_lane_u16(*(uint16_t *)src[15], src_v_upper, 7);

    // Gather the even and odd elements from the upper and lower halves into two vectors
    uint8x16_t src_evens = vuzp1q_u8(vreinterpretq_u8_u16(src_v_lower), vreinterpretq_u8_u16(src_v_upper));
    uint8x16_t src_odds = vuzp2q_u8(vreinterpretq_u8_u16(src_v_lower), vreinterpretq_u8_u16(src_v_upper));

    /*
     * Treat groups of elements as uint64_t's
     * a_tl = <u64> [0, 2, 4, ..., 14]
     * a_tr = <u64> [1, 3, 5, ..., 15]
     * a_bl = <u64> [16, 18, ..., 30]
     * a_br = <u64> [17, 19, ..., 31]
     */
    register uint64_t a_tl = vdupd_laneq_u64(vreinterpretq_u64_u8(src_evens), 0);
    register uint64_t a_tr = vdupd_laneq_u64(vreinterpretq_u64_u8(src_odds), 0);
    register uint64_t a_bl = vdupd_laneq_u64(vreinterpretq_u64_u8(src_evens), 1);
    register uint64_t a_br = vdupd_laneq_u64(vreinterpretq_u64_u8(src_odds), 1);

    // Scopes to attempt to force register use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask01(a_tl);
            register uint64_t col_bl = _bext_u64_mask01(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[0][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask02(a_tl);
            register uint64_t col_bl = _bext_u64_mask02(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[1][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask04(a_tl);
            register uint64_t col_bl = _bext_u64_mask04(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[2][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask08(a_tl);
            register uint64_t col_bl = _bext_u64_mask08(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[3][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask10(a_tl);
            register uint64_t col_bl = _bext_u64_mask10(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[4][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask20(a_tl);
            register uint64_t col_bl = _bext_u64_mask20(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[5][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask40(a_tl);
            register uint64_t col_bl = _bext_u64_mask40(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[6][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _bext_u64_mask80(a_tl);
            register uint64_t col_bl = _bext_u64_mask80(a_bl);
            col_l = _bdep_u64_maskff(col_tl) | _bdep_u64_maskff00(col_bl);
        }
        ((uint64_t**)targ)[7][0] |= (uint64_t)col_l;
    }


    
    // Scope to permit register re-use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask01(a_tr);
            register uint64_t col_br = _bext_u64_mask01(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[8][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask02(a_tr);
            register uint64_t col_br = _bext_u64_mask02(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[9][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask04(a_tr);
            register uint64_t col_br = _bext_u64_mask04(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[10][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask08(a_tr);
            register uint64_t col_br = _bext_u64_mask08(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[11][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask10(a_tr);
            register uint64_t col_br = _bext_u64_mask10(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[12][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask20(a_tr);
            register uint64_t col_br = _bext_u64_mask20(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[13][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask40(a_tr);
            register uint64_t col_br = _bext_u64_mask40(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[14][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _bext_u64_mask80(a_tr);
            register uint64_t col_br = _bext_u64_mask80(a_br);
            col_l = _bdep_u64_maskff(col_tr) | _bdep_u64_maskff00(col_br);
        }
        ((uint64_t**)targ)[15][0] |= (uint64_t)col_l;
    }
    
    return;
}
void simd_transpose_2x16(uint8_t** src, uint8_t** targ)
{
    __inline_simd_transpose_2x16(src, targ); 
}


void simd_transpose_64x64(uint64_t* restrict block_a[64], uint64_t* restrict block_b[64])
{
     uint64_t src_block[64] = {0};
     uint64_t targ_block[64] = {0};
    
     uint64_t* src_ptr[16];
     uint64_t* targ_ptr[16] = {NULL};


    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(targ_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_a[i + 16 * col])) + row); 
            }
            simd_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }


    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(src_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_b[i + 16 * col])) + row); 
            }
            simd_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }

    for (size_t i = 0; i < 64; i++)
    {
        memcpy(block_a[i], src_block + i, 8); 
        memcpy(block_b[i], targ_block + i, 8); 
    } 
 
    return;
}
void simd_transpose_64x64_inplace(uint64_t* block_a[64])
{
     uint64_t targ_block[64] = {0};
    
     uint64_t* restrict src_ptr[16];
     uint64_t* restrict targ_ptr[16] = {NULL};


    #pragma GCC unroll 4
    for (size_t col = 0; col < 4; col++) 
    {
        #pragma GCC unroll 4
        for (size_t row = 0; row < 4; row++)  
        {

            #pragma GCC unroll 16 
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(targ_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_a[i + 16 * col])) + row); 
            }
            simd_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);                     
        }
    }
    for (size_t i = 0; i < 64; i++)
    {
        memcpy(block_a[i], targ_block + i, 8); 
    } 
 
    return;
}

/*
 * Scalar implementation of the simd transpose
 * Used for regression testing
 */
static inline
void __inline_chunk_transpose_2x16(uint8_t** restrict src, uint8_t** restrict targ)
{
    for (size_t row = 0; row < 16; row++)  
    {
        uint64_t dispersed_vals = 0;
        for (size_t i = 0; i < 16; i++)  
        {
            dispersed_vals |= (!!(((uint16_t**)src)[i][0] & (1 << row))) << i; 
        }
        *((uint16_t*)(targ[row])) = dispersed_vals;
    } 
}
void __attribute__((noinline))
chunk_transpose_2x16(uint8_t** restrict src, uint8_t** restrict targ)
{
    __inline_chunk_transpose_2x16(src, targ); 
}

void __attribute__((noinline))
chunk_transpose_64x64(uint64_t* restrict block_a[64], uint64_t* restrict block_b[64])
{
     uint64_t src_block[64] = {0};
     uint64_t targ_block[64] = {0};
    
     uint64_t* src_ptr[16];
     uint64_t* targ_ptr[16] = {NULL};

    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(targ_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_a[i + 16 * col])) + row); 
            }
            chunk_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }

    for (size_t col = 0; col < 4; col++) 
    {
        for (size_t row = 0; row < 4; row++)  
        {
            #pragma GCC unroll 16
            for (size_t i = 0; i < 16; i++)
            {
                targ_ptr[i] = (uint64_t*)(((uint16_t*)(src_block + i + 16 * row)) + col);
                src_ptr[i] = (uint64_t*)(((uint16_t*)(block_b[i + 16 * col])) + row); 
            }
            chunk_transpose_2x16((uint8_t**)src_ptr, (uint8_t**)targ_ptr);
        }
    }

    for (size_t i = 0; i < 64; i++)
    {
        memcpy(block_a[i], src_block + i, 8); 
        memcpy(block_b[i], targ_block + i, 8); 
    } 
 
    return;
}


static inline
uint8_t __inline_get_bit(
    uint64_t* slice,
    const size_t index)
{
    uint64_t mask = 1ull << (index % 64);
    return !!(slice[index / 64] & mask); 
}

void __inline_set_bit(
    uint64_t* slice,
    const size_t index,
    const uint8_t value)
{
    slice[index / 64] &= ~(1ull << (index % 64)); 
    slice[index / 64] |= (1ull & value) << (index % 64); 
}

void transpose_naive(uint64_t** restrict block, const size_t size)
{

    for (size_t i = 0; i < size; i++)
    {
        // Inner loop should run along the current orientation, and hence along the cache lines 
        uint64_t* ptr = block[i]; 
    
        for (size_t j = i + 1; j < size; j++)
        {
            uint8_t val_a = __inline_get_bit(ptr, j); 
            uint8_t val_b = __inline_get_bit(block[j], i); 

            __inline_set_bit(ptr, j, val_b);
            __inline_set_bit(block[j], i, val_a);
        }    
    }
    return;
}


