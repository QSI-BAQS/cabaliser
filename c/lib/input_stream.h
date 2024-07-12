#ifndef INPUT_STREAM_H
#define INPUT_STREAM_H

#include "instructions.h"
#include "widget.h"

struct qubit_map_t {
    size_t n_qubits;
    size_t max_qubits;
    size_t* map;
};
typedef struct qubit_map_t qubit_map_t;


#endif
