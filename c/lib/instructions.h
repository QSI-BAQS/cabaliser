#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "consts.h"

#define MAX_INSTRUCTION_SEQUENCE_LEN (8)
#define CLIFFORD_OPCODE_WIDTH (1)

#define I_MASK    0b00 
#define H_MASK    0b01
#define P_MASK    0b10 
#define Pd_MASK   0b11

#define X_MASK    0b100 
#define Z_MASK    0b101
#define Y_MASK    0b110

#define CNOT_MASK 0b1000 
#define CZ_MASK   0b1001

#define RZ_MASK   0b10000000

/*
 * instruction_struct
 * Simple struct that allows for easy instruction parsing  
 */
struct instruction_t
{
    uint8_t opcode;
};
typedef struct instruction_t instruction_t;


/*
 * single qubit instruction struct
 *
 */
struct single_qubit_instruction
{
    uint8_t opcode;
    uint32_t arg;
};
struct two_qubit_instruction
{
    uint8_t opcode;
    uint32_t ctrl;
    uint32_t targ;
};
struct rz_instruction
{
    uint8_t opcode;
    uint32_t arg;
    float angle;  // TODO: Confirm that floating point precision is sufficient, else tag this
};

/* 
 * instruction_stream
 * Union of instruction types for readability
 * All types are aliased on the first field   
 * This first field is the opcode, which can be used as a distinguisher   
 */
union instruction_stream
{
    struct instruction_t instruction;
    struct single_qubit_instruction single;
    struct two_qubit_instruction multi;
    struct rz_instruction rz; 
};
typedef union instruction_stream instruction_stream_u;


/*
 * instruction_queue
 * Implemented a fixed length queue of single qubit Clifford instructions
 * Single qubit additions to the queue simply twirl the queue 
 * Two qubit additions to the queue either:
 *   - Commute through and twirl performing a local operation on the graph state 
 *   - Trigger a dump of the instructions to the tableau 
 */
struct instruction_queue
{
    size_t n_qubits; // Number of qubits
    instruction_t** table; // Queued instructions for each qubit 
};
typedef struct instruction_queue instruction_queue_t;

/*
 * instruction_queue_create
 * Constructor for an instruction queue
 * :: n_qubits : const size_t :: Number of qubits supported by the queue
 * Queue is allocated on the heap and requires the us of the destructor function instruction_queue_destroy
 */
instruction_queue_t* instruction_queue_create(const size_t n_qubits);

/*
 * instruction_queue_destroy
 * Destructor for an instruction queue 
 * :: que : instruction_queue_t* :: Instruction queue to be deallocated
 * Frees queue related memory from the heap
 */
void instruction_queue_destroy(instruction_queue_t* que);

#endif
