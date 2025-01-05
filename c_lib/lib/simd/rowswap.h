#ifndef ROWSWAP_H
#define ROWSWAP_H

#include "simd_headers.h"

void simd_row_swap(
    const size_t n_bytes,
    void* restrict ctrl_x, 
    void* restrict ctrl_z, 
    void* restrict targ_x, 
    void* restrict targ_z 
);


#endif
