#define INSTRUCTIONS_SRC // Loads tables from the header
#include "instructions.h"


/*
 * clifford_queue_create
 * Constructor for an instruction queue
 * :: n_qubits : const size_t :: Number of qubits supported by the queue
 * Queue is allocated on the heap and requires the us of the destructor function clifford_queue_destroy
 */
clifford_queue_t* clifford_queue_create(const size_t n_qubits)
{
    void* instructions = NULL;
    const size_t instruction_table_size = n_qubits * MAX_INSTRUCTION_SEQUENCE_LEN * CLIFFORD_OPCODE_WIDTH; 
    posix_memalign(&instructions, CACHE_SIZE, instruction_table_size); 

    memset(instructions, 0x00, instruction_table_size); 

    clifford_queue_t* que = NULL; 
    posix_memalign((void**)&que, CACHE_SIZE,  sizeof(clifford_queue_t));
    que->table = instructions;
    que->n_qubits = n_qubits;
    
    return que;
}


/*
 * instruction_process
 * Processes an incoming instruction
 * 
 */
bool instruction_process(clifford_queue_t* que, instruction_t inst)
{
  
    return 0;
}


/*
 * clifford_queue_destroy
 * Destructor for an instruction queue 
 * :: que : clifford_queue_t* :: Instruction queue to be deallocated
 * Frees queue related memory from the heap
 */
void clifford_queue_destroy(clifford_queue_t* que)
{
    free(que->table);
    free(que);
}


/*
 * local_clifford
 * Applies a local Clifford to the queue
 */
static inline
void __inline_local_clifford(
    widget_t* wid,
    struct single_qubit_instruction* inst)
{
    size_t idx = wid->q_map[inst->arg]; 
    wid->queue->table[idx] = SINGLE_QUBIT_CLIFFORD_MAP[inst->opcode][wid->queue->table[idx]]; 
} 
void local_clifford(
    widget_t* wid,
    struct single_qubit_instruction* inst)
{
    __inline_local_clifford(wid, inst);
}
