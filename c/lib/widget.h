#ifndef WIDGET_H
#define WIDGET_H

#include "tableau.h"
#include "instructions.h"
#include "input_stream.h"


struct widget_t {
    size_t max_qubits;
    size_t n_qubits;
    tableau_t* tableau;
    clifford_queue_t* queue;
    qubit_map_t* q_map; 
};
typedef struct widget_t widget_t;

#endif
