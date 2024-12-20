#ifndef SIMD_ROWSUM_H
#define SIMD_ROWSUM_H

#include <string.h>

#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <x86gprintrin.h>

#include "tableau.h"

#define ROWSUM_STRIDE (256 / 8) 

/*
 * simd_rowsum 
 * Performs a rowsum between two stabilisers 
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: void* :: Control X vec 
 * :: ctrl_z :: void* :: Control Z vec 
 * :: targ_x :: void* :: Target X vec 
 * :: targ_z :: void* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_rowsum(
    const size_t n_bytes,
    void* restrict ctrl_x,
    void* restrict ctrl_z,
    void* restrict targ_x,
    void* restrict targ_z
);

/*
 * simd_xor_rowsum 
 * Performs a rowsum between two stabilisers 
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: void* :: Control X vec 
 * :: ctrl_z :: void* :: Control Z vec 
 * :: targ_x :: void* :: Target X vec 
 * :: targ_z :: void* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_xor_rowsum(
    const size_t n_bytes,
    void* restrict ctrl_x,
    void* restrict ctrl_z,
    void* restrict targ_x,
    void* restrict targ_z
);

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
    void* restrict targ_z
);

/*
 * rowsum_cnf
 * Non-vectorised CNF implementation 
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: void* :: Control X vec 
 * :: ctrl_z :: void* :: Control Z vec 
 * :: targ_x :: void* :: Target X vec 
 * :: targ_z :: void* :: Target Z vec 
 * Returns the phase term 
 */
int8_t rowsum_cnf(
    const size_t n_bytes,
    void* restrict ctrl_x,
    void* restrict ctrl_z,
    void* restrict targ_x,
    void* restrict targ_z
);

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
    void* restrict ctrl_x,
    void* restrict ctrl_z,
    void* restrict targ_x,
    void* restrict targ_z
);

/*
 * simd_rowsum_cnf
 * Vectorised CNF implementation using popcnt 
 * :: n_bytes : size_t :: Length of the chunk 
 * :: ctrl_x :: void* :: Control X vec 
 * :: ctrl_z :: void* :: Control Z vec 
 * :: targ_x :: void* :: Target X vec 
 * :: targ_z :: void* :: Target Z vec 
 * Returns the phase term 
 */
int8_t simd_rowsum_cnf(
    const size_t n_bytes,
    void* restrict ctrl_x,
    void* restrict ctrl_z,
    void* restrict targ_x,
    void* restrict targ_z
);


#endif
