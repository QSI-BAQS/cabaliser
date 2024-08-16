#include "widget.h"

/*
 * widget_create
 * Constructor for widget object
 * :: max_qubits : const size_t :: Maximum number of qubits that may be allocated 
 *
 */
widget_t* widget_create(const size_t initial_qubits, const size_t max_qubits)
{
    widget_t* wid = malloc(sizeof(widget_t));  
    wid->n_qubits = initial_qubits;
    wid->max_qubits = max_qubits; 
    wid->tableau = tableau_create(max_qubits);
    wid->queue = clifford_queue_create(max_qubits);
    wid->q_map = qubit_map_create(initial_qubits, max_qubits); 

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


/*
 * widget_decompose
 * Decomposes the stabiliser tableau into a graph state plus local Cliffords 
 * :: wid : widget_t* :: Widget to decompose 
 * Acts in place on the tableau 
 */
void widget_decompose(widget_t* wid)
{
    tableau_remove_zero_X_columns(wid->tableau, wid->queue);

    tableau_transpose(wid->tableau);

    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (0 == __inline_slice_get_bit(wid->tableau->slices_x[i], i))
        {
            tableau_X_diag_element(wid->tableau, wid->queue, i);
        }

        tableau_X_diag_col_upper(wid->tableau, i);

    }


    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (0 == __inline_slice_get_bit(wid->tableau->slices_x[i], i))
        {
            tableau_X_diag_element(wid->tableau, wid->queue, i);
        }
        tableau_X_diag_col_lower(wid->tableau, i);
    }


    // Phase operation to set Z diagonal to zero 
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
        {
            // TODO transverse phase
            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
        }
    }

    // Z to set phases to 0
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->phases, i))
        {
            // TODO transverse Z 
            __inline_slice_set_bit(wid->tableau->phases, i, 0);
        }
    }

    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        if (__inline_slice_get_bit(wid->tableau->slices_z[i], i))
        {
            // TODO transverse phase
            __inline_slice_set_bit(wid->tableau->slices_z[i], i, 0);
        }
    }

    return;
}
