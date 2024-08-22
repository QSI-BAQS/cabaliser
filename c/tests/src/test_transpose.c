#include <string.h>

#include "simd_transpose.h"

int main()
{
    const size_t n_channels = 16;
    const size_t stride = 256;

    uint64_t* arr_a = NULL;

    uint64_t* arr_b_chunk = NULL;
    uint64_t* arr_b_simd = NULL;

    // Set up aligned memory
    posix_memalign((void**)&arr_a, 64, sizeof(uint64_t) * stride * n_channels);  
    posix_memalign((void**)&arr_b_chunk, 64, sizeof(uint64_t) * stride * n_channels);  
    posix_memalign((void**)&arr_b_simd, 64, sizeof(uint64_t) * stride * n_channels);  

    // Flush the memory 
    memset(arr_a, 0, sizeof(uint64_t) * stride * n_channels); 
    memset(arr_b_chunk, 0, sizeof(uint64_t) * stride * n_channels); 
    memset(arr_b_simd, 0, sizeof(uint64_t) * stride * n_channels); 


    uint64_t* ptrs_a[16];     
    uint64_t* ptrs_b_chunk[16];     
    uint64_t* ptrs_b_simd[16];     


    for (size_t i = 0; i < n_channels; i++)
    {
        ptrs_a[i] = arr_a + (i * stride); 
        ptrs_b_chunk[i] = arr_b_chunk + (i * stride); 
        ptrs_b_simd[i] = arr_b_simd + (i * stride); 
    }

    // Setting up the test
    // Left Column
    for (size_t i = 0; i < 8; i++)
    {
        ptrs_a[0][0] |= 1 << i;
    }

    // Right block offset 1 
    for (size_t i = 0; i < 8; i++)
    {
        ptrs_a[i + 8][0] |= 2;// << (i + 8);
    }

    // Below block 
    for (size_t i = 0; i < 8; i++)
    {
        ptrs_a[3][0] |= 1 << (i + 8);
    }

    // Diagonal
    for (size_t i = 1; i < 8; i++)
    {
        ptrs_a[i][0] |= (1 << (i - 1));
    }


    // Lower left block
    for (size_t i = 8; i < 16; i++)
    {
        ptrs_a[i][0] |= (1 << (i - 1));
    }

    simd_transpose_2x16((uint8_t**)ptrs_a, (uint8_t**)ptrs_b_simd);
    chunk_transpose_2x16((uint8_t**)ptrs_a, (uint8_t**)ptrs_b_chunk);

    for (size_t i = 0; i < 16; i++)
    {
        assert(ptrs_b_chunk[i][0] == ptrs_b_simd[i][0]);
    }

    free(arr_a);
    free(arr_b_simd);
    free(arr_b_chunk);


    return 0;
}
