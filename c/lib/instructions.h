#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "tableau.h"
#include "consts.h"
#include "widget.h"

#define MAX_INSTRUCTION_SEQUENCE_LEN (8)
#define CLIFFORD_OPCODE_WIDTH (1)

#define LOCAL_CLIFFORD_MASK (1 << 5) 
#define NON_LOCAL_CLIFFORD_MASK (1 << 6) 
#define RZ_MASK (1 << 7) 


struct clifford_queue_t;

/*
 * instruction_struct
 * Simple struct that allows for easy instruction parsing  
 */
typedef uint8_t instruction_t;
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
    instruction_t instruction;
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
struct clifford_queue_t
{
    size_t n_qubits; // Number of qubits
    instruction_t* table; // Queued instructions for each qubit 
    non_clifford_tag_t* non_clifford; // Terminating non-clifford instructions 
};
typedef struct clifford_queue_t clifford_queue_t;

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



// Only applies to the instructions src file
#ifdef INSTRUCTIONS_SRC

#define _I_ (0x00 | LOCAL_CLIFFORD_MASK)
#define _X_ (0x01 | LOCAL_CLIFFORD_MASK)
#define _Y_ (0x02 | LOCAL_CLIFFORD_MASK)
#define _Z_ (0x03 | LOCAL_CLIFFORD_MASK)
#define _H_ (0x04 | LOCAL_CLIFFORD_MASK)
#define _S_ (0x05 | LOCAL_CLIFFORD_MASK)
#define _R_ (0x06 | LOCAL_CLIFFORD_MASK)
#define _HX_ (0x07 | LOCAL_CLIFFORD_MASK)
#define _SX_ (0x08 | LOCAL_CLIFFORD_MASK)
#define _RX_ (0x09 | LOCAL_CLIFFORD_MASK)
#define _HY_ (0x0a | LOCAL_CLIFFORD_MASK)
#define _HZ_ (0x0b | LOCAL_CLIFFORD_MASK)
#define _SH_ (0x0c | LOCAL_CLIFFORD_MASK)
#define _RH_ (0x0d | LOCAL_CLIFFORD_MASK)
#define _HS_ (0x0e | LOCAL_CLIFFORD_MASK)
#define _HR_ (0x0f | LOCAL_CLIFFORD_MASK)
#define _HSX_ (0x10 | LOCAL_CLIFFORD_MASK)
#define _HRX_ (0x11 | LOCAL_CLIFFORD_MASK)
#define _SHY_ (0x12 | LOCAL_CLIFFORD_MASK)
#define _RHY_ (0x13 | LOCAL_CLIFFORD_MASK)
#define _HSH_ (0x14 | LOCAL_CLIFFORD_MASK)
#define _HRH_ (0x15 | LOCAL_CLIFFORD_MASK)
#define _RHS_ (0x16 | LOCAL_CLIFFORD_MASK)
#define _SHR_ (0x17 | LOCAL_CLIFFORD_MASK)

#define _CZ_ (0x00 | NON_LOCAL_CLIFFORD_MASK) 
#define _CNOT_ (0x01 | NON_LOCAL_CLIFFORD_MASK) 
#define _RZ_ (RZ_MASK)

const instruction_t SINGLE_QUBIT_CLIFFORD_MAP[7][24] = {
    /* I */ {_I_, _X_, _Y_, _Z_, _H_, _S_, _R_, _HX_, _SX_, _RX_, _HY_, _HZ_, _SH_, _RH_, _HS_, _HR_, _HSX_, _HRX_, _SHY_, _RHY_, _HSH_, _HRH_, _RHS_, _SHR_},
    /* X */ {_X_, _I_, _Z_, _Y_, _HZ_, _RX_, _SX_, _HY_, _R_, _S_, _HX_, _H_, _SHY_, _RHY_, _HR_, _HS_, _HRX_, _HSX_, _SH_, _RH_, _HRH_, _HSH_, _SHR_, _RHS_},
    /* Y */ {_Y_, _Z_, _I_, _X_, _HY_, _SX_, _RX_, _HZ_, _S_, _R_, _H_, _HX_, _RHY_, _SHY_, _HSX_, _HRX_, _HS_, _HR_, _RH_, _SH_, _RHS_, _SHR_, _HSH_, _HRH_},
    /* Z */ {_Z_, _Y_, _X_, _I_, _HX_, _R_, _S_, _H_, _RX_, _SX_, _HZ_, _HY_, _RH_, _SH_, _HRX_, _HSX_, _HR_, _HS_, _RHY_, _SHY_, _SHR_, _RHS_, _HRH_, _HSH_},
    /* H */ {_H_, _HX_, _HY_, _HZ_, _I_, _HS_, _HR_, _X_, _HSX_, _HRX_, _Y_, _Z_, _HSH_, _HRH_, _S_, _R_, _SX_, _RX_, _SHR_, _RHS_, _SH_, _RH_, _RHY_, _SHY_},
    /* S */ {_S_, _SX_, _RX_, _R_, _SH_, _Z_, _I_, _RH_, _Y_, _X_, _SHY_, _RHY_, _HX_, _H_, _HRH_, _SHR_, _HSH_, _RHS_, _HZ_, _HY_, _HR_, _HRX_, _HS_, _HSX_},
    /* R */ {_R_, _RX_, _SX_, _S_, _RH_, _I_, _Z_, _SH_, _X_, _Y_, _RHY_, _SHY_, _H_, _HX_, _RHS_, _HSH_, _SHR_, _HRH_, _HY_, _HZ_, _HSX_, _HS_, _HRX_, _HR_}
};
#endif

#endif
