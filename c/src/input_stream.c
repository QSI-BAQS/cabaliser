#include "input_stream.h"

/*
 * local_clifford_gate
 * Applies a local Clifford operation to the widget
 * Local in this context implies a single qubit operation
 * :: wid : widget_t* :: The widget
 * :: inst : single_qubit_instruction_t* :: The local Clifford operation 
 * Acts in place on the widget, should only update one entry in the local Clifford table 
 */
static inline
void __inline_local_clifford_gate(
    widget_t* wid,
    struct single_qubit_instruction* inst)
{
    size_t target = wid->q_map[inst->arg]; 
    wid->queue->table[target] = LOCAL_CLIFFORD_LEFT(inst->opcode, wid->queue->table[target]);

    // TODO Pauli Correction
    return;
} 


/*
 * apply_local_cliffords
 * Empties the local clifford table and applies the local cliffords
 * :: wid : widget_t* :: The widget
 * Acts in place over the tableau and the clifford table
 */
void apply_local_cliffords(widget_t* wid)
{
    for (size_t i = 0; i < wid->n_qubits; i++)
    {
        SINGLE_QUBIT_OPERATIONS[wid->queue->table[i] & INSTRUCTION_OPERATOR_MASK](wid->tableau, i);
        wid->queue->table[i] = _I_; 
    }
}


/*
 * non_local_clifford_gate
 * Applies a non-local Clifford operation to the widget
 * Local in this context implies a single qubit operation
 * :: wid : widget_t* :: The widget
 * :: inst : single_qubit_instruction_t* :: The local Clifford operation 
 * Acts in place on the widget, should only update one entry in the local Clifford table 
 */
static inline
void __inline_non_local_clifford_gate(
    widget_t* wid,
    struct two_qubit_instruction* inst)
{
    // First pass dump the tables and implement the instruction
    size_t ctrl = wid->q_map[inst->ctrl]; 
    size_t targ = wid->q_map[inst->targ]; 

    // Execute the queued cliffords 
    // TODO : Merge these operations with CX/CZ gates to cut runtime to one third 
    SINGLE_QUBIT_OPERATIONS[wid->queue->table[ctrl] & INSTRUCTION_OPERATOR_MASK](wid->tableau, ctrl);
    wid->queue->table[ctrl] = _I_;

    SINGLE_QUBIT_OPERATIONS[wid->queue->table[targ] & INSTRUCTION_OPERATOR_MASK](wid->tableau, targ);
    wid->queue->table[targ] = _I_;
    TWO_QUBIT_OPERATIONS[inst->opcode & INSTRUCTION_OPERATOR_MASK](wid->tableau, ctrl, targ);

    // TODO Pauli correction
    return;
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
    
    // This could be handled with a better return
    assert(wid->n_qubits < wid->max_qubits);

    const size_t ctrl = WMAP_LOOKUP(wid, inst->arg);
    const size_t targ = wid->n_qubits; 

    wid->queue->non_cliffords[ctrl] = inst->tag;
    wid->q_map[inst->arg] = wid->n_qubits;


    SINGLE_QUBIT_OPERATIONS[wid->queue->table[ctrl] & INSTRUCTION_OPERATOR_MASK](wid->tableau, ctrl);
    wid->queue->table[ctrl] = _I_;

    SINGLE_QUBIT_OPERATIONS[wid->queue->table[targ] & INSTRUCTION_OPERATOR_MASK](wid->tableau, targ);
    wid->queue->table[targ] = _I_;

    tableau_CNOT(wid->tableau, ctrl, targ);

    wid->n_qubits += 1;  

    // TODO Pauli Correction
    return;
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

        // The switch is the masked opcode, picking the three highest bits
        switch ((instructions + i)->instruction & INSTRUCTION_TYPE_MASK)  
        {
        case LOCAL_CLIFFORD_MASK:
            __inline_local_clifford_gate(wid, (struct single_qubit_instruction*)(instructions + i)); 
            break; 
        case NON_LOCAL_CLIFFORD_MASK:
            __inline_non_local_clifford_gate(wid, (struct two_qubit_instruction*)(instructions + i)); 
            break;
        case RZ_MASK:
            // Non-local Clifford operation
            __inline_rz_gate(wid, (struct rz_instruction*) instructions + i);
            break;
        }
    }
    return;
}
