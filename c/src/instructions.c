#include "instructions.h"

/*
 * instruction_queue_create
 * Constructor for an instruction queue
 */
instruction_queue_t* instruction_queue_create(const size_t n_qubits)
{
    void* instructions = NULL;
    const size_t instruction_table_size = n_qubits * MAX_INSTRUCTION_SEQUENCE_LEN * CLIFFORD_OPCODE_WIDTH; 
    posix_memalign(&instructions, CACHE_SIZE,instruction_table_size); 
    memset(instructions, 0x00, instruction_table_size); 

    instruction_queue_t* que = malloc(sizeof(instruction_queue_t));
    que->table = instructions;
    que->n_qubits = n_qubits;
    
    return que;
}


/*
 * instruction_queue_destroy
 * Destructor for an instruction queue 
 */
void instruction_queue_destroy(instruction_queue_t* que)
{
    free(que->table);
    free(que);
}
