#include "input_stream.h"

/*
 * map_qubit
 * Tracks the current mapping from circuit level operations to graph state operations
 * :: q_map : qubit_map_t* :: The current qubit map
 * :: index : size_t :: The index of the qubit to query
 * Returns the index after teleportation
 */
static inline
size_t __inline_map_qubit(
    qubit_map_t* q_map,
    size_t index)
{
    return q_map->map[index];
}




/*
 * rz_gate 
 * Implements an rz gate as a terminating operation
 * :: wid : widget_t* :: The widget in question 
 * :: inst : rz_instruction* :: The rz instruction indicating an angle 
 * Teleports an RZ operation, allocating a new qubit in the process
 * Compilation fails if the number of qubits exceeds some maximum 
 */
static inline
void __inline_rz_gate(
    widget_t* wid,
    struct rz_instruction* inst) 
{
    wid->n_qubits += 1;  

    // This could be handled with a better return
    assert(wid->n_qubits < wid->max_qubits);

    size_t idx = wid->q_map->map[inst->arg];

    wid->queue->non_clifford[idx] = inst->tag;
    wid->q_map->map[idx] = wid->n_qubits;
    wid->q_map->n_qubits = wid->n_qubits;

    return;
}


/*
 * local_clifford
 * Applies a local Clifford operation to the widget
 * Local in this context implies a single qubit operation
 * :: wid : widget_t* :: The widget
 * :: inst : single_qubit_instruction_t* :: The local Clifford operation 
 * Acts in place on the widget, should only update one entry in the local Clifford table 
 */
static inline
void __inline_local_clifford(
    widget_t* wid,
    struct single_qubit_instruction* inst)
{
   //size_t idx = wid->q_map->map[inst->arg];
   // TODO map
   //__inline_clifford_map(wid->queue->table[idx], inst->opcode);
}

/*
 * parse_instruction_block
 * Parses a block of instructions 
 * :: wid : widget_t* :: Current widget 
 * :: instructions : instruction_stream_u* :: Array of instructions 
 * :: n_instructions : const size_t :: Number of instructions in the stream 
 * This implementation may be replaced with a thread pool dispatcher
 */
void parse_instruction_block(
    widget_t* wid,
    instruction_stream_u* instructions,
    const size_t n_instructions)
{
    #pragma GCC unroll 8
    for (size_t i = 0; i < n_instructions; i++)
    {
        uint8_t opcode = instructions[i].instruction.opcode; 
        if (LOCAL_CLIFFORD_MASK & opcode)
        {
            // Local Clifford operation
        } 
        else if (NON_LOCAL_CLIFFORD_MASK & instructions[i].instruction.opcode)
        {
            // Non-local Clifford operation
        }
        else if (RZ_MASK & instructions[i].instruction.opcode)
        {
            // Non-local Clifford operation
            __inline_rz_gate(wid, (struct rz_instruction*) instructions + i);
        }
    } 
    return;
} 



