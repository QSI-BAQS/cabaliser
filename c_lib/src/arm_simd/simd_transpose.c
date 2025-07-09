#include "simd_transpose.h"

/*
 * Interal bit manipulation functions (naive)
 *
 * ARM Neon lacks ext, dup, etc. so either;
 *      1. SVE is required
 *      2. Software versions must be used
 */

static inline uint64_t __naive_bext_u64(uint64_t targ, uint64_t mask)
{
    uint64_t res = 0;
    int r_pos = 0;

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



// Transposes two blocks
static inline
void __inline_simd_transpose_2x16(uint8_t** restrict src, uint8_t** restrict targ)
{
    // Goal : load [1-31 : 2] into one vector and [0-30 : 2] into another
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

    uint8x16_t src_evens = vreinterpretq_u8_u16(vtrn1q_u16(src_v_lower, src_v_upper));
    uint8x16_t src_odds = vreinterpretq_u8_u16(vtrn2q_u16(src_v_lower, src_v_upper));

    register uint64_t a_tl = vdupd_laneq_u64(vreinterpretq_u64_u8(src_odds), 0);
    register uint64_t a_tr = vdupd_laneq_u64(vreinterpretq_u64_u8(src_odds), 1);
    register uint64_t a_bl = vdupd_laneq_u64(vreinterpretq_u64_u8(src_evens), 0);
    register uint64_t a_br = vdupd_laneq_u64(vreinterpretq_u64_u8(src_evens), 1);

    // Scopes to attempt to force register use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x0101010101010101ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x0101010101010101ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[0][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x0202020202020202ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x0202020202020202ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[1][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x0404040404040404ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x0404040404040404ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[2][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x0808080808080808ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x0808080808080808ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[3][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x1010101010101010ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x1010101010101010ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[4][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x2020202020202020ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x2020202020202020ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[5][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x4040404040404040ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x4040404040404040ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[6][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = __naive_bext_u64(a_tl, 0x8080808080808080ull);
            register uint64_t col_bl = __naive_bext_u64(a_bl, 0x8080808080808080ull);
            col_l = __naive_bdep_u64(col_tl, 0x00000000000000ffull) | __naive_bdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[7][0] |= (uint64_t)col_l;
    }


    
    // Scope to permit register re-use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x0101010101010101ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x0101010101010101ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[8][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x0202020202020202ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x0202020202020202ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[9][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x0404040404040404ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x0404040404040404ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[10][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x0808080808080808ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x0808080808080808ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[11][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x1010101010101010ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x1010101010101010ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[12][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x2020202020202020ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x2020202020202020ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[13][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x4040404040404040ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x4040404040404040ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)targ)[14][0] |= (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = __naive_bext_u64(a_tr, 0x8080808080808080ull);
            register uint64_t col_br = __naive_bext_u64(a_br, 0x8080808080808080ull);
            col_l = __naive_bdep_u64(col_tr, 0x00000000000000ffull) | __naive_bdep_u64(col_br, 0x000000000000ff00ull);
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


