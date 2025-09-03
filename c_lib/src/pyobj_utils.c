#include "pyobj_utils.h"


void pauli_string_conv(uint8_t* restrict  src, char* restrict  dst, const size_t len)
{
    // TODO: Potential speedup using avx masking here
    const static char table[4] = {'I', 'Z', 'X', 'Y'};  
    for (size_t i = 0; i < len; i++) 
    {
        dst[i] = table[src[i]]; 
    }
    return;
}

/*
 * Conditional measurement tag
 * This function acts as a getter to ensure that the value of the measurement tag is exposed to Python
 */
int conditional_measurement_tag()
{
    return BARE_MEASUREMENT_TAG;
}
