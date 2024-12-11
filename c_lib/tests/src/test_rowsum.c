#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "simd_rowsum.h"
#include "debug.h"

/*
 * Copies a len bit vector
 */
void* cpy_vec(void* vec, const size_t len)
{
    void* dst = malloc(len / 8); 
    memcpy(dst, vec, len / 8); 
    return dst;
}

/*
 * Creates a random len bit vector
 */
void* rand_vec(const size_t len)
{
    void* vec = malloc(len / 8); 

    for (size_t i = 0; i < len / 8; i++)
    {
        ((uint8_t*)vec)[i] = (uint8_t)rand();
    } 
    return vec;
}


/*
 * Creates a empty len bit vector
 */
void* empty_vec(const size_t len)
{
    void* vec = malloc(len / 8); 
    memset(vec, 0, len / 8);

    return vec;
}


void test_non_phase_terms(
    int8_t (*rowsum)(size_t, void*, void*, void*, void*),
    const size_t len)
{
    void* ctrl_x = rand_vec(len); 
    void* ctrl_z = rand_vec(len); 
    void* targ_x = rand_vec(len); 
    void* targ_z = rand_vec(len); 

    void* init_x = cpy_vec(targ_x, len);
    void* init_z = cpy_vec(targ_z, len);

    rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    for (size_t i = 0; i < len / 8; i++)
    {
        ((uint8_t*)targ_x)[i] ^= ((uint8_t*)ctrl_x)[i]; 
        assert(((uint8_t*)targ_x)[i] == ((uint8_t*)init_x)[i]);
        ((uint8_t*)targ_z)[i] ^= ((uint8_t*)ctrl_z)[i]; 
        assert(((uint8_t*)targ_z)[i] == ((uint8_t*)init_z)[i]);

    }  
    
    free(ctrl_x);
    free(ctrl_z);
    free(targ_x);
    free(targ_z);

    free(init_x);
    free(init_z);

}

void test_phase_terms(
    int8_t (*rowsum)(size_t, void*, void*, void*, void*),
    size_t len,
    size_t byte_shift,
    size_t bit_shift)
{
    uint8_t* ctrl_x = empty_vec(len); 
    uint8_t* ctrl_z = empty_vec(len); 
    uint8_t* targ_x = empty_vec(len); 
    uint8_t* targ_z = empty_vec(len); 

    // Empty test
    // Test 0 0 0 0
    DPRINT(DEBUG_1, "TESTING: 0 0 0 0\n");
    int8_t phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(0 == phase);

    // Test 1 0 0 0 
    DPRINT(DEBUG_1, "TESTING: 1 0 0 0\n");
    ctrl_x[byte_shift] = 1 << bit_shift; 
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    printf("%d %d %d %d %d\n", phase, *ctrl_x, *ctrl_z, *targ_x, *targ_z);
    assert(0 == phase);

    // Test 1 0 1 0
    DPRINT(DEBUG_1, "TESTING: 1 0 1 0\n");
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(0 == phase);

    // Test 1 0 0 1
    DPRINT(DEBUG_1, "TESTING: 1 0 0 1\n");
    targ_z[byte_shift] = 1 << bit_shift; 
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    printf("%d %d %d %d %d\n", phase, *ctrl_x, *ctrl_z, *targ_x, *targ_z);

    assert(-1 == phase);
    
    // Test 1 0 1 1
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(1 == phase);

    ////////
    // Test 1 1 0 1 
    ctrl_z[byte_shift] = 1 << bit_shift; 
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(1 == phase);

    // Test 1 1 1 0
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(-1 == phase);

    // Test 1 1 1 1
    targ_x[byte_shift] = 1 << bit_shift; 
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(0 == phase);
    
    // Test 1 1 0 0
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(0 == phase);

    ////////
    // Test 0 1 1 1 
    ctrl_x[byte_shift] = 0; 
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(-1 == phase);

    // Test 0 1 1 0
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(1 == phase);

    // Test 0 1 0 1
    targ_x[byte_shift] = 0; 
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(0 == phase);
    
    // Test 0 1 0 0
    phase = rowsum(len / 8, ctrl_x, ctrl_z, targ_x, targ_z);
    assert(0 == phase);

}

int main()
{
    const size_t n_fns = 2;
    int8_t (*fns[2])(size_t, void* restrict, void* restrict, void* restrict, void* restrict) = {simd_xor_rowsum, rowsum_naive_lookup_table};
    for (size_t i = 0; i < 1000; i++)
    {
        for (size_t j = 0; j < n_fns; j++)
        { 
            srand(i);
            test_non_phase_terms(fns[j], 256 * i);
            test_phase_terms(fns[j], 256, 0, 0);
        }
    }
    return 0;
}
