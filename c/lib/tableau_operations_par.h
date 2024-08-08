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
  const void (*SINGLE_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op) = {
        tableau_par_I, 
        tableau_par_X, 
        tableau_par_Y, 
        tableau_par_Z, 
        tableau_par_H, 
        tableau_par_S, 
        tableau_par_R, 
        tableau_par_HX,
        tableau_par_SX,
        tableau_par_RX,
        tableau_par_HY,
        tableau_par_HZ,
        tableau_par_SH,
        tableau_par_RH,
        tableau_par_HS,
        tableau_par_HR,
        tableau_par_HSX,
        tableau_par_HRX,
        tableau_par_SHY,
        tableau_par_RHY,
        tableau_par_HSH,
        tableau_par_HRH,
        tableau_par_RHS,
        tableau_par_SHR};

    const void (*TWO_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op) = {
        tableau_par_CNOT,
        tableau_par_CZ
};

#else

    extern const void (*SINGLE_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op);     
    extern const void (*TWO_QUBIT_PAR_OPERATIONS[N_LOCAL_CLIFFORDS])(struct distributed_tableau_op* op); 
#endif

#endif
