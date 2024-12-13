#ifndef SIMD_GAUSSIAN_ELIMINATION_H
#define SIMD_GAUSSIAN_ELIMINATION_H

#include "tableau_operations.h"


/*
 * simd_tableau_idx_swap_transverse 
 * Swaps indicies over both the X and Z slices  
 * Also swaps associated phases
 * :: tab : tableau_t* :: Tableau object to swap over 
 * :: i :: const size_t :: Index to swap 
 * :: j :: const size_t :: Index to swap
 * Acts in place on the tableau 
 */
void simd_tableau_idx_swap_transverse(tableau_t* restrict tab, const size_t i, const size_t j);


#endif


