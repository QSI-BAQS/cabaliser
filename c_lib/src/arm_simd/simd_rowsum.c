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
#define MNS ((int16_t) -1) 

#define NAIVE_LOOKUP_TABLE 0, 0, 0, 0, 0, 0, 1, 3, 0, 3, 0, 1, 0, 1, 3, 0

#define ROWSUM_MASK MASK_0, MASK_0, MASK_0, MASK_0

// Shuffle lookup table        00   01   10   11                             
#define ROWSUM_SHUFFLE_MASK_00 NIL, NIL, NIL, NIL
#define ROWSUM_SHUFFLE_MASK_01 NIL, NIL, PLS, MNS 
#define ROWSUM_SHUFFLE_MASK_10 NIL, MNS, NIL, PLS 
#define ROWSUM_SHUFFLE_MASK_11 NIL, PLS, MNS, NIL

#define ROWSUM_SHUFFLE_SEQ ROWSUM_SHUFFLE_MASK_00, ROWSUM_SHUFFLE_MASK_01, ROWSUM_SHUFFLE_MASK_10, ROWSUM_SHUFFLE_MASK_11
#define ROWSUM_SHUFFLE_MASK ROWSUM_SHUFFLE_SEQ, ROWSUM_SHUFFLE_SEQ 

/*          Structure of naive implementation is:
 *          if ((b_ctrl_x == 1) && (b_ctrl_z == 0))
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
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: CHUNK_OBJ* :: Control X vec 
 * :: ctrl_z :: CHUNK_OBJ* :: Control Z vec 
 * :: targ_x :: CHUNK_OBJ* :: Target X vec 
 * :: targ_z :: CHUNK_OBJ* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_rowsum(
    const size_t n_bytes,
    void* restrict ctrl_x, 
    void* restrict ctrl_z, 
    void* restrict targ_x,
    void* restrict targ_z 
)
{
    uint64_t mask_vals[] = { MASK_0, MASK_0 };
    uint8x16_t mask = vreinterpretq_u8_u64(vld1q_u64(mask_vals));

    uint8_t lookup_vals[] = { ROWSUM_SHUFFLE_SEQ };
    uint8x16_t lookup = vld1q_u8(lookup_vals);

    uint8x16_t accumulator = vdupq_n_u8(0);

    for (size_t i = 0; i < n_bytes; i += ROWSUM_STRIDE)
    {
        // Load vecs   
        uint16x8_t v_ctrl_x = vld1q_u16(
            ctrl_x + i 
        );

        uint16x8_t v_ctrl_z = vld1q_u16(
            ctrl_z + i 
        );

        uint16x8_t v_targ_x = vld1q_u16(
            targ_x + i 
        );

        uint16x8_t v_targ_z = vld1q_u16(
            targ_z + i 
        );

        // Perform XOR operations
        vst1q_u16(
            targ_x + i,
            veorq_u16(
                v_ctrl_x,
                v_targ_x
            )
        ); 

        vst1q_u16(
            targ_z + i,
            veorq_u16(
                v_ctrl_z,
                v_targ_z
            )
        );

        // Strictly required to unroll the shifts
        // Without this pragma the variables are not 
        // correctly treated as immediates and the code
        // will fail to compile  
        #pragma GCC unroll 8
        for (uint8_t j = 0; j < 8; j++)
        {   
            uint16x8_t lane = vandq_u16( // Apply the mask
                        vreinterpretq_u16_u8(mask), 
                        vshrq_n_u16(v_ctrl_x, j) // Shift right by j
                    ); 

            lane = vorrq_u16( // Or with existing lane  
                vshlq_n_u16( // Shift left by POS_CTRL_Z 
                    vandq_u16( // Apply the mask
                        vreinterpretq_u16_u8(mask), 
                        vshrq_n_u16(v_ctrl_z, j)
                    ), 
                POS_CTRL_Z),
                lane
            );

            lane = vorrq_u16( // Or with existing lane  
                vshlq_n_u16( // Shift left by POS_CTRL_X 
                    vandq_u16( // Apply the mask
                        vreinterpretq_u16_u8(mask), 
                        vshrq_n_u16(v_targ_x, j)
                    ), 
                POS_TARG_X),
                lane
            );

            lane = vorrq_u16( // Or with existing lane  
                vshlq_n_u16( // Shift left by POS_CTRL_Z 
                    vandq_u16( // Apply the mask
                        vreinterpretq_u16_u8(mask), 
                        vshrq_n_u16(v_targ_z, j)
                    ), 
                POS_TARG_Z),
                lane
            );

            uint8x16_t lookup_res = vqtbl1q_u8(lookup, vreinterpretq_u8_u16(lane)); 

            // Accumulator overflow is just a mod 4 operation
            accumulator = vaddq_u8(accumulator, lookup_res);
        }
    }  


    // Load the accumulator back to regular memory
    int8_t acc[32];  
    vst1q_u8((void*)acc, accumulator);
    //_mm256_storeu_epi8(acc, accumulator);
    for (size_t i = 1; i < 32; i++)
    {
        acc[0] += acc[i];
    }
    return acc[0];
}


/*
 * simd_xor_rowsum 
 * Performs a rowsum between two rows of stabilisers 
 * This uses simd operations for the xor but not the phase accumulator
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: CHUNK_OBJ* :: Control X vec 
 * :: ctrl_z :: CHUNK_OBJ* :: Control Z vec 
 * :: targ_x :: CHUNK_OBJ* :: Target X vec 
 * :: targ_z :: CHUNK_OBJ* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_xor_rowsum(
    const size_t n_bytes,
    void* restrict ctrl_x, 
    void* restrict ctrl_z, 
    void* restrict targ_x, 
    void* restrict targ_z 
)
{
    int16_t acc = 0;
    for (size_t i = 0; i < n_bytes; i += ROWSUM_STRIDE)
    {

        // Load vecs   
        uint8x16_t v_ctrl_x = vld1q_u8(
            ((void*)ctrl_x) + i 
        );

        uint8x16_t v_ctrl_z = vld1q_u8(
            ((void*)ctrl_z) + i 
        );

        uint8x16_t v_targ_x = vld1q_u8(
            ((void*)targ_x) + i 
        );

        uint8x16_t v_targ_z = vld1q_u8(
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
        v_targ_x = veorq_u8(
            v_ctrl_x,
            v_targ_x
        ); 
        v_targ_z = veorq_u8(
            v_ctrl_z,
            v_targ_z
        ); 
       
        // Store target values  
        vst1q_u8(
            ((void*)targ_x) + i,
            v_targ_x 
        );
        vst1q_u8(
            ((void*)targ_z) + i,
            v_targ_z 
        );

        acc %= 4;
    }  

    return acc;
}


void simd_rowsum_xor_only(
    const size_t n_bytes,
    void* restrict ctrl_x, 
    void* restrict ctrl_z, 
    void* restrict targ_x, 
    void* restrict targ_z 
)
{
    for (size_t i = 0; i < n_bytes; i += ROWSUM_STRIDE)
    {
        uint8x16_t v_targ_x = vld1q_u8(
            targ_x + i 
        );

        uint8x16_t v_ctrl_x = vld1q_u8(
            ctrl_x + i 
        );

        // Perform XOR operations
        vst1q_u8(
            targ_x + i,
            veorq_u8(
                v_ctrl_x,
                v_targ_x
            )
        ); 

        uint8x16_t v_ctrl_z = vld1q_u8(
            ctrl_z + i 
        );

        uint8x16_t v_targ_z = vld1q_u8(
            targ_z + i 
        );
        
        // Perform XOR operations
        vst1q_u8(
            targ_z + i,
            veorq_u8(
                v_ctrl_z,
                v_targ_z
            )
        ); 
    }
}


/*
 * rowsum_naive_lookup_table
 * Naive lookup table implementation
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: void* :: Control X vec 
 * :: ctrl_z :: void* :: Control Z vec 
 * :: targ_x :: void* :: Target X vec 
 * :: targ_z :: void* :: Target Z vec 
 * Returns the phase term 
 */
int8_t rowsum_naive_lookup_table(
    const size_t n_bytes,
    void* restrict ctrl_x,
    void* restrict ctrl_z,
    void* restrict targ_x,
    void* restrict targ_z)
{

    uint8_t lookup_table[16] = {
        NAIVE_LOOKUP_TABLE
    };

    uint32_t sum = 0;

    for (size_t i = 0; i < n_bytes * 8; i++) {

        int idx = (
              (((((uint8_t*)ctrl_x)[i / 8] >> (i % 8)) & 0x1) << 3)
            | (((((uint8_t*)ctrl_z)[i / 8] >> (i % 8)) & 0x1) << 2)
            | (((((uint8_t*)targ_x)[i / 8] >> (i % 8)) & 0x1) << 1)
            | (((((uint8_t*)targ_z)[i / 8] >> (i % 8)) & 0x1) << 0)
        );

        sum += lookup_table[idx];
    }

    simd_rowsum_xor_only(n_bytes, ctrl_x, ctrl_z, targ_x, targ_z);
    return ((sum + 2) % 4 - 2);
}


int8_t rowsum_cnf(
    const size_t n_bytes,
    void* restrict ctrl_x, 
    void* restrict ctrl_z, 
    void* restrict targ_x, 
    void* restrict targ_z) 
{
    uint64_t pos = 0;
    uint64_t neg = 0;

    for (size_t i = 0; i < n_bytes / sizeof(uint64_t); i++) {
        uint64_t plus = 0;
        uint64_t minus = 0;

        plus = (~((uint64_t*)ctrl_x)[i] & ((uint64_t*)ctrl_z)[i] & ((uint64_t*)targ_x)[i] & ~((uint64_t*)targ_z)[i]) 
                | (((uint64_t*)ctrl_x)[i] & ~((uint64_t*)ctrl_z)[i] & ((uint64_t*)targ_x)[i] & ((uint64_t*)targ_z)[i]) 
                | (((uint64_t*)ctrl_x)[i] & ((uint64_t*)ctrl_z)[i] & ~((uint64_t*)targ_x)[i] & ((uint64_t*)targ_z)[i]);
        pos += __builtin_popcountll(plus);

        minus = (((uint64_t*)ctrl_x)[i] & ~((uint64_t*)ctrl_z)[i] & ~((uint64_t*)targ_x)[i] & ((uint64_t*)targ_z)[i]) 
                | (~((uint64_t*)ctrl_x)[i] & ((uint64_t*)ctrl_z)[i] & ((uint64_t*)targ_x)[i] & ((uint64_t*)targ_z)[i]) 
                | (((uint64_t*)ctrl_x)[i] & ((uint64_t*)ctrl_z)[i] & ((uint64_t*)targ_x)[i] & ~((uint64_t*)targ_z)[i]);
        neg += __builtin_popcountll(minus);
    }
    simd_rowsum_xor_only(n_bytes, ctrl_x, ctrl_z, targ_x, targ_z);
    return ((((pos - neg) % 4) + 2) % 4) - 2;
}

/*
 * simd_rowsum_cnf_popcnt
 * Vectorised CNF implementation using popcnt 
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: void* :: Control X vec 
 * :: ctrl_z :: void* :: Control Z vec 
 * :: targ_x :: void* :: Target X vec 
 * :: targ_z :: void* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_rowsum_cnf_popcnt(
    const size_t n_bytes,
    void *restrict ctrl_x,
    void *restrict ctrl_z,
    void *restrict targ_x,
    void *restrict targ_z) 
{
    uint64_t pos = 0;
    uint64_t neg = 0;
    for (size_t i = 0; i < n_bytes; i+=16) {
        uint8x16_t v_ctrl_x = vld1q_u8(ctrl_x + i);
        uint8x16_t v_ctrl_z = vld1q_u8(ctrl_z + i);
        uint8x16_t v_targ_x = vld1q_u8(targ_x + i);
        uint8x16_t v_targ_z = vld1q_u8(targ_z + i);

        uint8x16_t plus = vorrq_u8(
        vandq_u8(vmvnq_u8(vorrq_u8(v_targ_z, v_ctrl_x)), vandq_u8(v_ctrl_z, v_targ_x)),
        vandq_u8(vandq_u8(v_ctrl_x, v_targ_z), veorq_u8(v_ctrl_z, v_targ_x)));
        
        uint8x16_t minus = vorrq_u8(
        vandq_u8(vmvnq_u8(vorrq_u8(v_targ_x, v_ctrl_z)), vandq_u8(v_ctrl_x, v_targ_z)),
        vandq_u8(vandq_u8(v_ctrl_z, v_targ_x), veorq_u8(v_ctrl_x, v_targ_z)));

        pos += vaddvq_u8(vcntq_u8(plus));

        neg += vaddvq_u8(vcntq_u8(minus)); 
    }

    simd_rowsum_xor_only(n_bytes, ctrl_x, ctrl_z, targ_x, targ_z);

    return (((pos - neg) % 4 + 2) % 4) - 2;
}

int8_t simd_rowsum_cnf(
    const size_t n_bytes,
    void *restrict ctrl_x,
    void *restrict ctrl_z,
    void *restrict targ_x,
    void *restrict targ_z) 
{
    uint8x16_t pos = vmovq_n_u8(0);
    uint8x16_t neg = vmovq_n_u8(0);
    uint8x16_t acc = vmovq_n_u8(0);

    for (size_t i = 0; i < n_bytes; i += 16) {
        uint8x16_t v_ctrl_x = vld1q_u8(ctrl_x + i);
        uint8x16_t v_ctrl_z = vld1q_u8(ctrl_z + i);
        uint8x16_t v_targ_x = vld1q_u8(targ_x + i);
        uint8x16_t v_targ_z = vld1q_u8(targ_z + i);

        uint8x16_t plus = vorrq_u8(
            vandq_u8(
                vmvnq_u8(vorrq_u8(v_targ_z, v_ctrl_x)),
                vandq_u8(v_ctrl_z, v_targ_x)),
            vandq_u8(
                vandq_u8(v_ctrl_x, v_targ_z),
                veorq_u8(v_ctrl_z, v_targ_x)
            ));
        
        uint8x16_t minus = vorrq_u8(
            vandq_u8(
                vmvnq_u8(vorrq_u8(v_targ_x, v_ctrl_z)),
                vandq_u8(v_ctrl_x, v_targ_z)),
            vandq_u8(
                vandq_u8(v_ctrl_z, v_targ_x),
                veorq_u8(v_ctrl_x, v_targ_z)
            ));
        
        // Perform XOR operations
        vst1q_u8(
            targ_x + i,
            veorq_u8(
                v_ctrl_x,
                v_targ_x
            )
        ); 

        vst1q_u8(
            targ_z + i,
            veorq_u8(
                v_ctrl_z,
                v_targ_z
            )
        );


        acc = veorq_u8(acc, vandq_u8(pos, plus));
        acc = veorq_u8(acc, vandq_u8(neg, minus));

        pos = veorq_u8(pos, plus);
        neg = veorq_u8(neg, minus);
    }

    uint64_t total = 0;

    total += vaddvq_u8(vcntq_u8(pos));

    total -= vaddvq_u8(vcntq_u8(neg));

    total += vaddvq_u8(vcntq_u8(acc)); 

    return ((total + 2) % 4) - 2;
}
