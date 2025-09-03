#ifndef PYOBJ_UTILS_H
#define PYOBJ_UTILS_H
#include <stddef.h>
#include <stdint.h>

#include "conditional_operations.h"

void pauli_string_conv(uint8_t* src, char* dst, const size_t len);

/*
 * Conditional measurement tag
 * This function acts as a getter to ensure that the value of the measurement tag is exposed to Python
 */
int conditional_measurement_tag();


#endif
