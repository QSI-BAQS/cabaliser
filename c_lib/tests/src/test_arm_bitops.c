/*
 * Tests the optimised versions of software bext/bdep for ARM
 */

#include <assert.h>
#include "simd_transpose.h"

// Naive generic bext
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

// Naive generic bdep
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


void test_bext_mask01()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x0101010101010101ull) == _bext_u64_mask01(rand_v));
    }
}

void test_bext_mask02()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x0202020202020202ull) == _bext_u64_mask02(rand_v));
    }
}


void test_bext_mask04()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x0404040404040404ull) == _bext_u64_mask04(rand_v));
    }
}


void test_bext_mask08()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x0808080808080808ull) == _bext_u64_mask08(rand_v));
    }
}

void test_bext_mask10()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x1010101010101010ull) == _bext_u64_mask10(rand_v));
    }
}


void test_bext_mask20()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x2020202020202020ull) == _bext_u64_mask20(rand_v));
    }
}


void test_bext_mask40()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x4040404040404040ull) == _bext_u64_mask40(rand_v));
    }
}


void test_bext_mask80()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bext_u64(rand_v, 0x8080808080808080ull) == _bext_u64_mask80(rand_v));
    }
}


void test_bdep_maskff()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bdep_u64(rand_v, 0x00000000000000ffull) == _bdep_u64_maskff(rand_v));
    }
}

void test_bdep_maskff00()
{
    for (int i = 0; i < 10000; i++)
    {
        uint64_t rand_v = rand();
        //printf("%lu\n", rand_v);
        assert(__naive_bdep_u64(rand_v, 0x000000000000ff00ull) == _bdep_u64_maskff00(rand_v));
    }
}


int main(void)
{
    srand(1);
    test_bext_mask01();
    //printf("\n-------------\n\n");
    test_bext_mask02();
    //printf("\n-------------\n\n");
    test_bext_mask04();
    //printf("\n-------------\n\n");
    test_bext_mask08();
    //printf("\n-------------\n\n");
    test_bext_mask10();
    //printf("\n-------------\n\n");
    test_bext_mask20();
    //printf("\n-------------\n\n");
    test_bext_mask40();
    //printf("\n-------------\n\n");
    test_bext_mask80();


    //printf("\n-------------\n\n");
    test_bdep_maskff();
    //printf("\n-------------\n\n");
    test_bdep_maskff00();

    return 0;
}
