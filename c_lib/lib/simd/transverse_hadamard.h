#ifndef TRANSVERSE_HADAMARD_H
#define TRANSVERSE_HADAMARD_H

#include "tableau.h"
#include "simd_headers.h"

void simd_tableau_transverse_hadamard(tableau_t const* tab, const size_t targ);

#endif
