#include "simd_rowsum.h"

#define MASK_0 (0x0101010101010101ull) 
#define MASK_1 (0x0202020202020202ull) 
#define MASK_2 (0x0404040404040404ull) 
#define MASK_3 (0x0808080808080808ull) 
#define MASK_4 (0x1010101010101010ull) 
#define MASK_5 (0x2020202020202020ull) 
#define MASK_6 (0x4040404040404040ull) 
#define MASK_7 (0x8080808080808080ull) 

#define POS_CTRL_X (0) 
#define POS_CTRL_Z (1) 
#define POS_TARG_X (2) 
#define POS_TARG_Z (3) 

#define NIL (0)
#define PLS (1)
#define MNS ((int8_t) -1) 

#define ROWSUM_MASK MASK_0, MASK_0, MASK_0, MASK_0

//                             00   01   10   11                             
#define ROWSUM_SHUFFLE_MASK_00 NIL, NIL, NIL, NIL
#define ROWSUM_SHUFFLE_MASK_01 NIL, NIL, NIL, NIL
#define ROWSUM_SHUFFLE_MASK_10 MNS, MNS, MNS, MNS
#define ROWSUM_SHUFFLE_MASK_11 NIL, NIL, NIL, NIL
#define ROWSUM_SHUFFLE_SEQ ROWSUM_SHUFFLE_MASK_00, ROWSUM_SHUFFLE_MASK_01, ROWSUM_SHUFFLE_MASK_10, ROWSUM_SHUFFLE_MASK_11
#define ROWSUM_SHUFFLE_MASK ROWSUM_SHUFFLE_SEQ, ROWSUM_SHUFFLE_SEQ 
/*
 *            if ((b_ctrl_x == 1) && (b_ctrl_z == 0))
 *          {
 *              acc += b_targ_z * (2 * b_targ_x - 1); 
 *          }
 *          else if ((b_ctrl_x == 0) && (b_ctrl_z == 1))
 *          {
 *              acc += b_targ_x * (1 - 2 * b_targ_z);
 *          }
 *          else if ((b_ctrl_x == 1) && (b_ctrl_z == 1))
 *          {
 *              acc += b_targ_z - b_targ_x;
 *          }
 */


/*
 * simd_rowsum 
 * Performs a rowsum between two rows of stabilisers 
 * :: slice_len : size_t :: Length of the chunk 
 * :: ctrl_x :: CHUNK_OBJ* :: Control X vec 
 * :: ctrl_z :: CHUNK_OBJ* :: Control Z vec 
 * :: targ_x :: CHUNK_OBJ* :: Target X vec 
 * :: targ_z :: CHUNK_OBJ* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_rowsum(
    size_t n_bytes,
    void* ctrl_x, 
    void* ctrl_z, 
    void* targ_x, 
    void* targ_z 
)
{
    __m256i mask = _mm256_setr_epi64x(ROWSUM_MASK); 
    __m256i lookup = _mm256_setr_epi8(ROWSUM_SHUFFLE_MASK); 
    __m256i accumulator = _mm256_setzero_si256(); 

    printf("%lu %d\n", n_bytes, ROWSUM_STRIDE);
    for (size_t i = 0; i < n_bytes; i += ROWSUM_STRIDE)
    {

        // Load vecs   
        __m256i v_ctrl_x = _mm256_loadu_si256(
            ctrl_x + i 
        );

        __m256i v_ctrl_z = _mm256_loadu_si256(
            ctrl_z + i 
        );

        __m256i v_targ_x = _mm256_loadu_si256(
            targ_x + i 
        );

        __m256i v_targ_z = _mm256_loadu_si256(
            targ_z + i 
        );

        printf("TARG_X\n");
        for (size_t i = 0; i < 4; i++)
        {
            printf("%llu," , v_targ_x[i]);
        }
        printf("\n");



        // Perform XOR operations
        _mm256_storeu_si256(
            targ_x + i,
            _mm256_xor_si256(
                v_ctrl_x,
                v_targ_x
            )
        ); 

        _mm256_storeu_si256(
            targ_z + i,
            _mm256_xor_si256(
                v_ctrl_z,
                v_targ_z
            )
        ); 

        // Strictly required to unroll the shifts
        // Without this pragma the variables are not 
        // correctly treated as immediates and the code
        // will fail to compile  
        #pragma GCC unroll 4
        for (uint8_t j = 0; j < 1; j++)
        {   
            // TODO shift
            __m256i lane = _mm256_and_si256(mask, v_ctrl_x); 
            printf("v_ctrl_x\n");
            for (size_t i = 0; i < 4; i++)
            {
                printf("%llu," , lane[i]);
            }
            printf("\n");



            lane = _mm256_or_si256( // Or with existing lane  
                _mm256_bslli_epi128( // Shift left by POS_CTRL_Z 
                    _mm256_and_si256( // Apply the mask
                        mask, 
                        _mm256_bsrli_epi128(v_ctrl_z, j)
                    ), 
                POS_CTRL_Z),
                lane
            );

            printf("v_ctrl_z\n");
            for (size_t i = 0; i < 4; i++)
            {
                printf("%llu," , lane[i]);
            }

            printf("\n");


            lane = _mm256_or_si256( // Or with existing lane  
                _mm256_bslli_epi128( // Shift left by POS_CTRL_Z 
                    _mm256_and_si256( // Apply the mask
                        mask, 
                        _mm256_bsrli_epi128(v_targ_x, j)
                    ), 
                POS_TARG_X),
                lane
            );

            printf("v_targ_x\n");
            for (size_t i = 0; i < 4; i++)
            {
                printf("%llu," , lane[i]);
            }
           printf("\n");


            lane = _mm256_or_si256( // Or with existing lane  
                _mm256_bslli_epi128( // Shift left by POS_CTRL_Z 
                    _mm256_and_si256( // Apply the mask
                        mask, 
                        _mm256_bsrli_epi128(v_targ_z, j)
                    ), 
                POS_TARG_Z),
                lane
            );

            printf("v_targ_z\n");
            for (size_t i = 0; i < 4; i++)
            {
                printf("%llu," , lane[i]);
            }
            printf("\n");

            //// Apply lookup 
            lookup = _mm256_shuffle_epi8(lookup, lane); 

            printf("Lookup\n");
            for (size_t i = 0; i < 4; i++)
            {
                printf("%llu," , lookup[i]);
            }
            printf("\n");


            // Accumulator overflow is just a mod 4 operation
            accumulator = _mm256_add_epi8(accumulator, lookup);  
        }
    }  

    // Load the accumulator back to regular memory
    int8_t acc[32];  
    _mm256_storeu_epi8(acc, accumulator);
    printf("%u, ", acc[0]);
    for (size_t i = 1; i < 32; i++)
    {
        printf("%u, ", acc[i]);
        acc[0] += acc[i];
    }   
    printf("\n");
    return acc[0];
}


/*
 * simd_xor_rowsum 
 * Performs a rowsum between two rows of stabilisers 
 * This uses simd operations for the xor but not the phase accumulator
 * :: slice_len : size_t :: Length of the chunk 
 * :: ctrl_x :: CHUNK_OBJ* :: Control X vec 
 * :: ctrl_z :: CHUNK_OBJ* :: Control Z vec 
 * :: targ_x :: CHUNK_OBJ* :: Target X vec 
 * :: targ_z :: CHUNK_OBJ* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_xor_rowsum(
    size_t n_bytes,
    void* ctrl_x, 
    void* ctrl_z, 
    void* targ_x, 
    void* targ_z 
)
{

    int16_t acc = 0;
    for (size_t i = 0; i < n_bytes; i += ROWSUM_STRIDE)
    {

        // Load vecs   
        __m256i v_ctrl_x = _mm256_loadu_si256(
            ((void*)ctrl_x) + i 
        );

        __m256i v_ctrl_z = _mm256_loadu_si256(
            ((void*)ctrl_z) + i 
        );

        __m256i v_targ_x = _mm256_loadu_si256(
            ((void*)targ_x) + i 
        );

        __m256i v_targ_z = _mm256_loadu_si256(
            ((void*)targ_z) + i 
        );

        // Goal: avoid dumping to stack    
        for (size_t j = 0; j < 256; j++)
        {
            int8_t b_ctrl_x = __inline_slice_get_bit(ctrl_x, j + i * 8);
            int8_t b_ctrl_z = __inline_slice_get_bit(ctrl_z, j + i * 8);
            int8_t b_targ_x = __inline_slice_get_bit(targ_x, j + i * 8);
            int8_t b_targ_z = __inline_slice_get_bit(targ_z, j + i * 8);

            if ((b_ctrl_x == 1) && (b_ctrl_z == 0))
            {
                acc += b_targ_z * (2 * b_targ_x - 1); 
            }
            else if ((b_ctrl_x == 0) && (b_ctrl_z == 1))
            {
                acc += b_targ_x * (1 - 2 * b_targ_z);
            }
            else if ((b_ctrl_x == 1) && (b_ctrl_z == 1))
            {
                acc += b_targ_z - b_targ_x;
            }
        }   
 
        // Perform XOR operations
        v_targ_x = _mm256_xor_si256(
            v_ctrl_x,
            v_targ_x
        ); 
        v_targ_z = _mm256_xor_si256(
            v_ctrl_z,
            v_targ_z
        ); 
       
        // Store target values  
        _mm256_storeu_si256(
            ((void*)targ_x) + i,
            v_targ_x 
        );
        _mm256_storeu_si256(
            ((void*)targ_z) + i,
            v_targ_z 
        );

        acc %= 4;
    }  

    return acc;
}
