#ifndef WIDGET_H
#define WIDGET_H

#include "tableau.h"
#include "instructions.h"
#include "qubit_map.h"

#define WMAP_LOOKUP(widget, idx) (widget->q_map[idx]) 

struct widget_t {
    size_t max_qubits;
    size_t n_qubits;
    struct tableau_t* tableau;
    struct clifford_queue_t* queue;
    qubit_map_t* q_map; 
};
typedef struct widget_t widget_t;


/*
 * widget_create
 * Constructor for widget object
 * :: max_qubits : const size_t :: Maximum number of qubits that may be allocated 
 *
 */
widget_t* widget_create(const size_t max_qubits);


/*
 * widget_destroy
 * Destructor for the widget object
 * :: wid : widget_t* :: widget to free
 */
void widget_destroy(widget_t* wid);


#endif
