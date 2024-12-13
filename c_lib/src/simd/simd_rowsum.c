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
 * :: slice_len : size_t :: Length of the chunk 
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
    __m256i mask = _mm256_setr_epi64x(ROWSUM_MASK); 
    __m256i lookup = _mm256_setr_epi8(ROWSUM_SHUFFLE_MASK); 
    __m256i accumulator = _mm256_setzero_si256(); 

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
        #pragma GCC unroll 8
        for (uint8_t j = 0; j < 8; j++)
        {   
            __m256i lane = _mm256_and_si256( // Apply the mask
                        mask, 
                        _mm256_srli_epi16(v_ctrl_x, j) // Shift right by j
                    ); 

            lane = _mm256_or_si256( // Or with existing lane  
                _mm256_slli_epi16( // Shift left by POS_CTRL_Z 
                    _mm256_and_si256( // Apply the mask
                        mask, 
                        _mm256_srli_epi16(v_ctrl_z, j)
                    ), 
                POS_CTRL_Z),
                lane
            );

            lane = _mm256_or_si256( // Or with existing lane  
                _mm256_slli_epi16( // Shift left by POS_CTRL_X 
                    _mm256_and_si256( // Apply the mask
                        mask, 
                        _mm256_srli_epi16(v_targ_x, j)
                    ), 
                POS_TARG_X),
                lane
            );

            lane = _mm256_or_si256( // Or with existing lane  
                _mm256_slli_epi16( // Shift left by POS_CTRL_Z 
                    _mm256_and_si256( // Apply the mask
                        mask, 
                        _mm256_srli_epi16(v_targ_z, j)
                    ), 
                POS_TARG_Z),
                lane
            );

            __m256i lookup_res = _mm256_shuffle_epi8(lookup, lane); 

            // Accumulator overflow is just a mod 4 operation
            accumulator = _mm256_add_epi8(accumulator, lookup_res);  
        }
    }  


    // Load the accumulator back to regular memory
    int8_t acc[32];  
    _mm256_storeu_si256((void*)acc, accumulator);
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
 * :: slice_len : size_t :: Length of the chunk 
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
        __m256i v_targ_x = _mm256_loadu_si256(
            targ_x + i 
        );

        __m256i v_ctrl_x = _mm256_loadu_si256(
            ctrl_x + i 
        );

        // Perform XOR operations
        _mm256_storeu_si256(
            targ_x + i,
            _mm256_xor_si256(
                v_ctrl_x,
                v_targ_x
            )
        ); 

        __m256i v_ctrl_z = _mm256_loadu_si256(
            ctrl_z + i 
        );

        __m256i v_targ_z = _mm256_loadu_si256(
            targ_z + i 
        );
        
        // Perform XOR operations
        _mm256_storeu_si256(
            targ_z + i,
            _mm256_xor_si256(
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
    size_t n_bits = n_bytes * 8;

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
    for (size_t i = 0; i < n_bytes; i+=32) {
        __m256i v_ctrl_x = _mm256_loadu_si256(ctrl_x + i);
        __m256i v_ctrl_z = _mm256_loadu_si256(ctrl_z + i);
        __m256i v_targ_x = _mm256_loadu_si256(targ_x + i);
        __m256i v_targ_z = _mm256_loadu_si256(targ_z + i);

        __m256i plus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(v_targ_z, v_ctrl_x), _mm256_and_si256(v_ctrl_z, v_targ_x)),
        _mm256_and_si256(_mm256_and_si256(v_ctrl_x, v_targ_z), _mm256_xor_si256(v_ctrl_z, v_targ_x)));
        
        __m256i minus = _mm256_or_si256(
        _mm256_andnot_si256(_mm256_or_si256(v_targ_x, v_ctrl_z), _mm256_and_si256(v_ctrl_x, v_targ_z)),
        _mm256_and_si256(_mm256_and_si256(v_ctrl_z, v_targ_x), _mm256_xor_si256(v_ctrl_x, v_targ_z)));

        pos += _mm_popcnt_u64(_mm256_extract_epi64(plus, 0))
            + _mm_popcnt_u64(_mm256_extract_epi64(plus, 1))
            + _mm_popcnt_u64(_mm256_extract_epi64(plus, 2))
            + _mm_popcnt_u64(_mm256_extract_epi64(plus, 3));

        neg += _mm_popcnt_u64(_mm256_extract_epi64(minus, 0))
            + _mm_popcnt_u64(_mm256_extract_epi64(minus, 1))
            + _mm_popcnt_u64(_mm256_extract_epi64(minus, 2))
            + _mm_popcnt_u64(_mm256_extract_epi64(minus, 3));
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
    __m256i pos = _mm256_setzero_si256();
    __m256i neg = _mm256_setzero_si256();
    __m256i acc = _mm256_setzero_si256();

    for (size_t i = 0; i < n_bytes; i += 32) {
        __m256i v_ctrl_x = _mm256_loadu_si256(ctrl_x + i);
        __m256i v_ctrl_z = _mm256_loadu_si256(ctrl_z + i);
        __m256i v_targ_x = _mm256_loadu_si256(targ_x + i);
        __m256i v_targ_z = _mm256_loadu_si256(targ_z + i);

        __m256i plus = _mm256_or_si256(
            _mm256_andnot_si256(
                _mm256_or_si256(v_targ_z, v_ctrl_x),
                _mm256_and_si256(v_ctrl_z, v_targ_x)),
            _mm256_and_si256(
                _mm256_and_si256(v_ctrl_x, v_targ_z),
                _mm256_xor_si256(v_ctrl_z, v_targ_x)
            ));
        
        __m256i minus = _mm256_or_si256(
            _mm256_andnot_si256(
                _mm256_or_si256(v_targ_x, v_ctrl_z),
                _mm256_and_si256(v_ctrl_x, v_targ_z)),
            _mm256_and_si256(
                _mm256_and_si256(v_ctrl_z, v_targ_x),
                _mm256_xor_si256(v_ctrl_x, v_targ_z)
            ));
        
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


        acc = _mm256_xor_si256(acc, _mm256_and_si256(pos, plus));
        acc = _mm256_xor_si256(acc, _mm256_and_si256(neg, minus));

        pos = _mm256_xor_si256(pos, plus);
        neg = _mm256_xor_si256(neg, minus);
    }

    uint64_t total = 0;

    total += _mm_popcnt_u64(_mm256_extract_epi64(pos, 0))
        + _mm_popcnt_u64(_mm256_extract_epi64(pos, 1))
        + _mm_popcnt_u64(_mm256_extract_epi64(pos, 2))
        + _mm_popcnt_u64(_mm256_extract_epi64(pos, 3));

    total -= _mm_popcnt_u64(_mm256_extract_epi64(neg, 0))
        + _mm_popcnt_u64(_mm256_extract_epi64(neg, 1))
        + _mm_popcnt_u64(_mm256_extract_epi64(neg, 2))
        + _mm_popcnt_u64(_mm256_extract_epi64(neg, 3));
    
    total += (_mm_popcnt_u64(_mm256_extract_epi64(acc, 0))
            + _mm_popcnt_u64(_mm256_extract_epi64(acc, 1))
            + _mm_popcnt_u64(_mm256_extract_epi64(acc, 2))
            + _mm_popcnt_u64(_mm256_extract_epi64(acc, 3))) << 1;

    return ((total + 2) % 4) - 2;
}
