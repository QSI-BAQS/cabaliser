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
    size_t n_bytes,
    void* ctrl_x, 
    void* ctrl_z, 
    void* targ_x, 
    void* targ_z
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
    size_t n_bytes,
    void* ctrl_x, 
    void* ctrl_z, 
    void* targ_x, 
    void* targ_z
);

#endif
