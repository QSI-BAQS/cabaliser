#include "instructions.h"


// Local Cache tables for fast Clifford lookups  
instruction_t SINGLE_QUBIT_CLIFFORD_TABLE[256];
instruction_t CNOT_CTRL_TABLE[256]; 
instruction_t CNOT_TARG_TABLE[256]; 
instruction_t CZ_TABLE[256]; 


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
