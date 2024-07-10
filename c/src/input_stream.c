#include "input_stream.h"


/*
 * parse_instruction_block
 * Parses a block of instructions 
 *
 */
void parse_instruction_block()
{
     

    return;
} 


static inline
size_t __inline_map_qubit(qubit_map_t* q_map, size_t index)
{
    return q_map->map[idx];
}

static inline
void __inline_rotation(widget_t* wid, rz_instruction* inst) 
{
    wid->n_qubits += 1;  

    // This could be handled with a better return
    assert(wid->n_qubits < max_qubits);

    size_t idx = wid->q_map->table[inst->arg];

    wid->queue->non_clifford[idx] = inst->angle;
    wid->q_map->map[idx] = wid->n_qubits;
    wid->q_map->n_qubits = wid->n_qubits;

    return;
}


static inline
void __inline_local_clifford(widget_t* wid, single_qubit_instruction* inst)
{

   size_t idx = wid->q_map->table[inst->arg];
   __inline_clifford_map(wid->queue->table[idx], inst->opcode);

}
