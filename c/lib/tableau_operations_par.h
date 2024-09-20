#ifndef TABLEAU_OPERATIONS_PAR_H
#define TABLEAU_OPERATIONS_PAR_H

#include "tableau.h"
#include "instructions.h"


#include "threadpool.h"

#include "tableau_operations.h"

#ifndef OPT_PRAGMA
#define OPT_PRAGMA
    #pragma GCC optimize("Ofast,unroll-loops")
    #pragma GCC target("avx2,tune=native")
#endif


/*
 * tableau_<clifford>
 * Single qubit clifford operations
 * :: tab : tableau_t* :: The tableau to operate on 
 * :: targ : const size_t :: The target qubit
 * Acts in place on the tableau
 */
void tableau_par_I(struct distributed_tableau_op* op); 
void tableau_par_X(struct distributed_tableau_op* op); 
void tableau_par_Y(struct distributed_tableau_op* op); 
void tableau_par_Z(struct distributed_tableau_op* op); 
void tableau_par_H(struct distributed_tableau_op* op); 
void tableau_par_S(struct distributed_tableau_op* op); 
void tableau_par_R(struct distributed_tableau_op* op); 
void tableau_par_HX(struct distributed_tableau_op* op); 
void tableau_par_SX(struct distributed_tableau_op* op); 
void tableau_par_RX(struct distributed_tableau_op* op); 
void tableau_par_HY(struct distributed_tableau_op* op); 
void tableau_par_HZ(struct distributed_tableau_op* op); 
void tableau_par_SH(struct distributed_tableau_op* op); 
void tableau_par_RH(struct distributed_tableau_op* op); 
void tableau_par_HS(struct distributed_tableau_op* op); 
void tableau_par_HR(struct distributed_tableau_op* op); 
void tableau_par_HSX(struct distributed_tableau_op* op);
void tableau_par_HRX(struct distributed_tableau_op* op);
void tableau_par_SHY(struct distributed_tableau_op* op);
void tableau_par_RHY(struct distributed_tableau_op* op);
void tableau_par_HSH(struct distributed_tableau_op* op);
void tableau_par_HRH(struct distributed_tableau_op* op);
void tableau_par_RHS(struct distributed_tableau_op* op);
void tableau_par_SHR(struct distributed_tableau_op* op);


void tableau_par_CNOT(struct distributed_tableau_op* op);
void tableau_par_CZ(struct distributed_tableau_op* op);

#ifdef TABLEAU_OPERATIONS_PAR_SRC

    // GCC has no issue without the void* casts, clang throws errors 
    const void (*SINGLE_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op) = {
        (void*)tableau_par_I, 
        (void*)tableau_par_X, 
        (void*)tableau_par_Y, 
        (void*)tableau_par_Z, 
        (void*)tableau_par_H, 
        (void*)tableau_par_S, 
        (void*)tableau_par_R, 
        (void*)tableau_par_HX,
        (void*)tableau_par_SX,
        (void*)tableau_par_RX,
        (void*)tableau_par_HY,
        (void*)tableau_par_HZ,
        (void*)tableau_par_SH,
        (void*)tableau_par_RH,
        (void*)tableau_par_HS,
        (void*)tableau_par_HR,
        (void*)tableau_par_HSX,
        (void*)tableau_par_HRX,
        (void*)tableau_par_SHY,
        (void*)tableau_par_RHY,
        (void*)tableau_par_HSH,
        (void*)tableau_par_HRH,
        (void*)tableau_par_RHS,
        (void*)tableau_par_SHR};

    const void (*TWO_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op) = {
        (void*)tableau_par_CNOT,
        (void*)tableau_par_CZ
};

#else

    extern const void (*SINGLE_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op);     
    extern const void (*TWO_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op); 
#endif

#endif
