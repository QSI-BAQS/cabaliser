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

    int err_code = posix_memalign(&instructions, CACHE_SIZE, n_qubits); 
    assert(0 == err_code);
    memset(instructions, _I_, n_qubits); 

    clifford_queue_t* que = NULL; 
    err_code = posix_memalign((void**)&que, CACHE_SIZE,  sizeof(clifford_queue_t));
    assert(0 == err_code);

    non_clifford_tag_t* non_cliffords = NULL;
    err_code = posix_memalign((void**)&non_cliffords, CACHE_SIZE,  n_qubits * sizeof(non_clifford_tag_t));
    assert(0 == err_code);
    memset(non_cliffords, 0, n_qubits * sizeof(non_clifford_tag_t)); 


    que->table = (instruction_t*)instructions;
    que->n_qubits = n_qubits;
    que->non_cliffords = (non_clifford_tag_t*)non_cliffords; 

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


/*
 * clifford_queue_local_clifford_right 
 * Applies clifford operator from the right of the expression  
 * This is used when extracting Clifford terms from the graph state preparation
 * :: que : clifford_queue_t* :: The clifford queue
 * :: cliff : instruction_t :: The instruction 
 * :: target : size_t :: The target qubit 
 */
void clifford_queue_local_clifford_right(clifford_queue_t* que, const instruction_t cliff, const size_t target)
{
    que->table[target] = LOCAL_CLIFFORD_RIGHT(cliff, que->table[target]);
}
