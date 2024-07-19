#include "widget.h"

/*
 * widget_create
 * Constructor for widget object
 * :: max_qubits : const size_t :: Maximum number of qubits that may be allocated 
 *
 */
widget_t* widget_create(const size_t max_qubits)
{
    widget_t* wid = malloc(sizeof(widget_t));  
    wid->n_qubits = 0;
    wid->max_qubits = max_qubits; 
    wid->tableau = tableau_create(max_qubits);
    wid->queue = clifford_queue_create(max_qubits);
    wid->q_map = qubit_map_create(max_qubits); 

    return wid;
}

/*
 * widget_destroy
 * Destructor for the widget object
 * :: wid : widget_t* :: widget to free
 */
void widget_destroy(widget_t* wid)
{
    tableau_destroy(wid->tableau);
    clifford_queue_destroy(wid->queue);
    qubit_map_destroy(wid->q_map);

    free(wid);
}
