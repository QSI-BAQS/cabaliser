#include "simd_transpose.h"

// Transposes two blocks
void 2x16_simd_transpose(uint8_tv** arr_a, uint8_tv** arr_b)
{

    static uint64_t tmp[64];    

    __m256i marr_a = _mm256_set_epi16(
            *(uint16_t*)arr_a[15],
            *(uint16_t*)arr_a[14],
            *(uint16_t*)arr_a[13],
            *(uint16_t*)arr_a[12],
            *(uint16_t*)arr_a[11],
            *(uint16_t*)arr_a[10],
            *(uint16_t*)arr_a[9],
            *(uint16_t*)arr_a[8],
            *(uint16_t*)arr_a[7],
            *(uint16_t*)arr_a[6],
            *(uint16_t*)arr_a[5],
            *(uint16_t*)arr_a[4],
            *(uint16_t*)arr_a[3],
            *(uint16_t*)arr_a[2],
            *(uint16_t*)arr_a[1],
            *(uint16_t*)arr_a[0]
            );     

    // Transpose high and low bytes
    // This shuffle is within each 128 byte lane
    __m256i shuffle_mask = _mm256_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15, 16, 18, 20, 22, 24, 26, 28, 30, 17, 19, 21, 23, 25, 27, 29, 31); 
    marr_a = _mm256_shuffle_epi8(marr_a.simdv, shuffle_mask); 
    
    // Transpose between 128 bit lanes
    marr_a = _mm256_permute4x64_epi64(marr_a, 0xd8);

    // bmi2 operations act 3x faster on registers
    register uint64_t a_tl = _mm_extract_epi64(__builtin_ia32_extract128i256(marr_a, 0), 0);
    register uint64_t a_tr = _mm_extract_epi64(__builtin_ia32_extract128i256(marr_a, 1), 0);
    register uint64_t a_bl = _mm_extract_epi64(__builtin_ia32_extract128i256(marr_a, 0), 1);
    register uint64_t a_br = _mm_extract_epi64(__builtin_ia32_extract128i256(marr_a, 1), 1);

    // Scopes to attempt to force register use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0101010101010101ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0101010101010101ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[0][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0202020202020202ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0202020202020202ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[1][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0404040404040404ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0404040404040404ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[2][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x0808080808080808ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x0808080808080808ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[3][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x1010101010101010ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x1010101010101010ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[4][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x2020202020202020ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x2020202020202020ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[5][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x4040404040404040ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x4040404040404040ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[6][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tl = _pext_u64(a_tl, 0x8080808080808080ull);
            register uint64_t col_bl = _pext_u64(a_bl, 0x8080808080808080ull);
            col_l = _pdep_u64(col_tl, 0x00000000000000ffull) | _pdep_u64(col_bl, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[7][0] = (uint64_t)col_l;
    }


    
    // Scope to permit register re-use 
    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0101010101010101ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0101010101010101ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[8][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0202020202020202ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0202020202020202ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[9][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0404040404040404ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0404040404040404ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[10][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x0808080808080808ull);
            register uint64_t col_br = _pext_u64(a_br, 0x0808080808080808ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[11][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x1010101010101010ull);
            register uint64_t col_br = _pext_u64(a_br, 0x1010101010101010ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[12][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x2020202020202020ull);
            register uint64_t col_br = _pext_u64(a_br, 0x2020202020202020ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[13][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x4040404040404040ull);
            register uint64_t col_br = _pext_u64(a_br, 0x4040404040404040ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[14][0] = (uint64_t)col_l;
    }

    {
        register uint64_t col_l;
        {
            register uint64_t col_tr = _pext_u64(a_tr, 0x8080808080808080ull);
            register uint64_t col_br = _pext_u64(a_br, 0x8080808080808080ull);
            col_l = _pdep_u64(col_tr, 0x00000000000000ffull) | _pdep_u64(col_br, 0x000000000000ff00ull);
        }
        ((uint64_t**)arr_b)[15][0] = (uint64_t)col_l;
    }

    return;
}





    return;
}


/*
 * Scalar implementation of the simd transpose
 * Used for regression testing
 */
void 2x16_chunk_transpose(uint64_t** arr_a, uint64_t** arr_b)
{
    for (size_t row = 0; row < 16; row++)  
    {
        uint64_t dispersed_vals = 0;
        for (size_t i = 0; i < 16; i++)  
        {
            dispersed_vals |= (!!(arr_a[i][0] & (1 << row))) << i; 
        }
        arr_b[row][0] = dispersed_vals;
    } 
}


