#include "qubit_map.h"

/*
 * qubit_map_create 
 * Constructor for a qubit map object
 * :: n_qubits : size_t :: Maximum number of qubits in the map 
 *
 */
qubit_map_t* qubit_map_create(const size_t initial_qubits, const size_t max_qubits)
{
    qubit_map_t* q_map = (qubit_map_t*)malloc(max_qubits * sizeof(size_t));

    for (size_t i = 0; i < initial_qubits; i++)
    {
        q_map[i] = i;
    }    
    return q_map;
}


/*
 * qubit_map_destroy
 * Destructor for the qubit map
 * :: q_map : qubit_map_t* :: The qubit map to free
 */
void qubit_map_destroy(qubit_map_t* q_map)
{
    free(q_map);    
}
