#define INSTRUCTIONS_TABLE // Loads tables from the header
#define INSTRUCTIONS_SRC 

#include "instructions.h"

/*
 * clifford_queue_create
 * Constructor for an instruction queue
 * :: n_qubits : const size_t :: Number of qubits supported by the queue
 * Queue is allocated on the heap and requires the use of the destructor function clifford_queue_destroy
 */
clifford_queue_t* clifford_queue_create(const size_t n_qubits)
{
    void* instructions = NULL;
    const size_t instruction_table_size = n_qubits; 
    posix_memalign(&instructions, CACHE_SIZE, instruction_table_size); 

    memset(instructions, _I_, instruction_table_size); 

    clifford_queue_t* que = NULL; 
    posix_memalign((void**)&que, CACHE_SIZE,  sizeof(clifford_queue_t));
    que->table = (instruction_t*)instructions;
    que->n_qubits = n_qubits;
    
    return que;
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

