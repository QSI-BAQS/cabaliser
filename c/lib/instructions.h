#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "consts.h"

#define MAX_INSTRUCTION_SEQUENCE_LEN (8)
#define CLIFFORD_OPCODE_WIDTH (1)

#define LOCAL_CLIFFORD_MASK 0b00100000  

#define I_MASK    0b00100000 
#define H_MASK    0b00100001
#define P_MASK    0b00100010 
#define Pd_MASK   0b00100011

#define X_MASK    0b00100100 
#define Z_MASK    0b00100101
#define Y_MASK    0b00100110

#define NON_LOCAL_CLIFFORD_MASK 0b01000000 
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


typedef uint32_t non_clifford_tag_t;


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
    uint32_t tag; // Tag for gate type, supports up to 2**32 unique non-Clifford gates per widget 
    // As each non-Clifford results in a teleportation, we should saturate memory bounds before 
    // this limit is reached   
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
 * clifford_queue
 * Implemented a fixed length queue of single qubit Clifford instructions
 * Single qubit additions to the queue simply twirl the queue 
 * Two qubit additions to the queue either:
 *   - Commute through and twirl performing a local operation on the graph state 
 *   - Trigger a dump of the instructions to the tableau 
 */
struct clifford_queue
{
    size_t n_qubits; // Number of qubits
    instruction_t* table; // Queued instructions for each qubit 
    non_clifford_tag_t* non_clifford; // Terminating non-clifford instructions 
};
typedef struct clifford_queue clifford_queue_t;

/*
 * clifford_queue_create
 * Constructor for an instruction queue
 * :: n_qubits : const size_t :: Number of qubits supported by the queue
 * Queue is allocated on the heap and requires the us of the destructor function clifford_queue_destroy
 */
clifford_queue_t* clifford_queue_create(const size_t n_qubits);

/*
 * clifford_queue_destroy
 * Destructor for an instruction queue 
 * :: que : clifford_queue_t* :: Instruction queue to be deallocated
 * Frees queue related memory from the heap
 */
void clifford_queue_destroy(clifford_queue_t* que);

#endif
