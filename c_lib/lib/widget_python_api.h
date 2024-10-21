#ifndef WIDGET_API_H
#define WIDGET_API_H

/*
 * Functions for use with the python API
 * Most of these accept a pointer that is handled by the python
 * scope and then write values to the appropriate array.
 *
 */

#include "widget.h"

/*
 * widget_get_adjacencies_api
 * For a qubit in the tableau, list all adjacent qubits 
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: target_qubit : const size_t :: The target qubit 
 * :: adj : struct adjacency_obj* :: Pointer to object to write to
 * Returns a heap allocated array of uint32_t objects
 */
void widget_get_adjacencies_api(const widget_t* wid, const size_t target_qubit, struct adjacency_obj* adj);


/*
 * widget_get_io_map_api
 * Returns an array of io map objects
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: arr : uint32_t** :: Pointer to io map to return 
 * Writes the pointer to the io map to *arr
 */
void widget_get_io_map_api(const widget_t* wid, qubit_map_t** arr);

/*
 * widget_get_measurements_api
 * Returns an array of measurements objects
 * :: wid : const widget_t* :: The widget to get adjacencies for 
 * :: arr : uint32_t** :: Pointer to io map to return 
 * Writes the pointer to the io map to *arr
 */
void widget_get_measurements_api(const widget_t* wid, non_clifford_tag_t** arr);

/*
 * wiget_get_pauli_tracker_api
 * Gets the pauli tracker pointer from the widget
 * :: wid : const widget_t* :: Widget pointer
 * :: tracker  : void** :: Address to write pointer to
 */
void widget_get_pauli_tracker_api(
    const widget_t* wid,
    void** tracker);

/*
 * widget_print_tableau
 * Prints the tableau
 */
void widget_print_tableau_api(const widget_t* wid);


#endif
