#ifndef QUBIT_MAP_H
#define QUBIT_MAP_H

#include <stdlib.h>


typedef size_t qubit_map_t;

#define MAP_LOOKUP(qubit_map, idx) (qubit_map[idx]) 
/*
 * create_qubit_map
 * Constructor for a qubit map object
 * :: n_qubits : size_t :: Maximum number of qubits in the map 
 *
 */
qubit_map_t* qubit_map_create(size_t n_qubits);

/*
 * qubit_map_destroy
 * Destructor for the qubit map
 * :: q_map : qubit_map_t* :: The qubit map to free
 */
void qubit_map_destroy(qubit_map_t* q_map);


#endif
