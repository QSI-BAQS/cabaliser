#include <string.h>

#include "simd_transpose.h"

void print(uint64_t** arr_a, uint64_t** arr_b, const size_t n_channels)
{
for (size_t i = 0; i < n_channels; i++)
{
    printf("|");
    for (size_t j = 0; j < n_channels; j++)
    {
        printf("%d ", !!(arr_a[j][0] & (1 << i)));
    }
    printf("| |");
    for (size_t j = 0; j < n_channels; j++)
    {
        printf("%d ", !!(arr_b[j][0] & (1 << i)));
    }
    printf("|\n");
} 
printf("\n");
}



void test_2x16()
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

    //    print(ptrs_a, ptrs_b_chunk, 16);
    simd_transpose_2x16((uint8_t**)ptrs_a, (uint8_t**)ptrs_b_simd);
    chunk_transpose_2x16((uint8_t**)ptrs_a, (uint8_t**)ptrs_b_chunk);
    print(ptrs_b_chunk, ptrs_b_simd, 16);

    for (size_t i = 0; i < 16; i++)
    {
        assert(ptrs_b_chunk[i][0] == ptrs_b_simd[i][0]);
    }

    free(arr_a);
    free(arr_b_simd);
    free(arr_b_chunk);

    return;
}



void test_single_chunks_64x64()
{
    const size_t n_channels = 64;
    const size_t stride = 1; 

    uint64_t* arr_a = NULL;
    uint64_t* arr_b_chunk = NULL;
    uint64_t* arr_b_simd = NULL;

    // Creating a single 64x64 block
    const size_t n_bytes = sizeof(uint64_t) * n_channels;
    const size_t col_size = sizeof(uint64_t);

    // Set up aligned memory
    posix_memalign((void**)&arr_a, 64, n_bytes);  
    posix_memalign((void**)&arr_b_chunk, 64, n_bytes);  
    posix_memalign((void**)&arr_b_simd, 64, n_bytes);  


    // Flush the memory 
    memset(arr_a, 0, n_bytes); 
    memset(arr_b_chunk, 0, n_bytes); 
    memset(arr_b_simd, 0, n_bytes); 

    // Initial state equal    
    assert(n_bytes == 8 * 64);
    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_b_simd)[i]);
    }

    uint64_t* ptrs_a[64] = {NULL};     
    uint64_t* ptrs_b_chunk[64] = {NULL};     
    uint64_t* ptrs_b_simd[64] = {NULL};     

    
    for (size_t i = 0; i < n_channels; i++)
    {
        ptrs_a[i] = arr_a + i;
        ptrs_b_chunk[i] = arr_b_chunk + i;
        ptrs_b_simd[i] = arr_b_simd + i;
    }

    for (size_t i = 0; i < n_bytes; i+=1)
    {
        ((uint8_t*)arr_a)[i] = (i | (i % 3)) | (i << (i % 7));
    }

    chunk_transpose_64x64(ptrs_a, ptrs_b_chunk);
    simd_transpose_64x64(ptrs_a, ptrs_b_simd);

    for (size_t i = 0; i < 16; i++)
    {
        print(ptrs_b_simd, ptrs_b_chunk, 16);
    }

    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_b_simd)[i]);
    }
}

void test_64x64()
{
    const size_t n_channels = 64;
    const size_t stride = 1; 

    uint64_t* arr_a = NULL;
    uint64_t* arr_b_chunk = NULL;
    uint64_t* arr_b_simd = NULL;

    // Creating a single 64x64 block
    const size_t n_bytes = sizeof(uint64_t) * n_channels;
    const size_t col_size = sizeof(uint64_t);

    // Set up aligned memory
    posix_memalign((void**)&arr_a, 64, n_bytes);  
    posix_memalign((void**)&arr_b_chunk, 64, n_bytes);  
    posix_memalign((void**)&arr_b_simd, 64, n_bytes);  

    // Flush the memory 
    memset(arr_a, 0, n_bytes); 
    memset(arr_b_chunk, 0, n_bytes); 
    memset(arr_b_simd, 0, n_bytes); 

    // Initial state equal    
    assert(n_bytes == 8 * 64);
    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_b_simd)[i]);
    }

    uint64_t* ptrs_a[64] = {NULL};     
    uint64_t* ptrs_b_chunk[64] = {NULL};     
    uint64_t* ptrs_b_simd[64] = {NULL};     
    
    for (size_t i = 0; i < n_channels; i++)
    {
        ptrs_a[i] = arr_a + i;
        ptrs_b_chunk[i] = arr_b_chunk + i;
        ptrs_b_simd[i] = arr_b_simd + i;
    }

    for (size_t i = 0; i < n_bytes; i+=1)
    {
        ((uint8_t*)arr_a)[i] = (i | (i % 3)) | (i << (i % 7));
    }

    simd_transpose_64x64(ptrs_a, ptrs_b_simd);
    chunk_transpose_64x64(ptrs_a, ptrs_b_chunk);
    
    for (size_t i = 0; i < n_bytes; i++)
    {
        assert(((uint8_t*)arr_b_chunk)[i] == ((uint8_t*)arr_b_simd)[i]);
    }
}

int main()
{
    test_2x16();
    test_64x64();
    return 0;
}
