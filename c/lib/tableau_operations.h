#ifndef TABLEAU_OPERATIONS_H
#define TABLEAU_OPERATIONS_H

#include "tableau.h"
#include "instructions.h"

/*
 * tableau_remove_zero_X_columns
 * Uses hadamards to clear any zeroed X columns
 * :: tab : tableau_t* :: The tableau to act on
 * :: c_que : clifford_queue_t* :: The clifford queue
 * Acts in place over the tableau
 */
void tableau_remove_zero_X_columns(tableau_t* tab, clifford_queue_t* c_que);


/*
 * tableau_Z_block_diagonal
 * Ensures that the Z block of the tableau is diagonal
 * :: tab : tableau_t* :: Tableau to act on
 * :: c_que : clifford_queue_t* :: The clifford queue
 * Acts in place over the tableau   
 */
void tableau_Z_zero_diagonal(tableau_t* tab, clifford_queue_t* c_que);


/*
 *
 *
 *
 */
void tableau_I(tableau_t* tab, const size_t targ); 
void tableau_X(tableau_t* tab, const size_t targ); 
void tableau_Y(tableau_t* tab, const size_t targ); 
void tableau_Z(tableau_t* tab, const size_t targ); 
void tableau_H(tableau_t* tab, const size_t targ); 
void tableau_S(tableau_t* tab, const size_t targ); 
void tableau_R(tableau_t* tab, const size_t targ); 
void tableau_HX(tableau_t* tab, const size_t targ); 
void tableau_SX(tableau_t* tab, const size_t targ); 
void tableau_RX(tableau_t* tab, const size_t targ); 
void tableau_HY(tableau_t* tab, const size_t targ); 
void tableau_HZ(tableau_t* tab, const size_t targ); 
void tableau_SH(tableau_t* tab, const size_t targ); 
void tableau_RH(tableau_t* tab, const size_t targ); 
void tableau_HS(tableau_t* tab, const size_t targ); 
void tableau_HR(tableau_t* tab, const size_t targ); 
void tableau_HSX(tableau_t* tab, const size_t targ);
void tableau_HRX(tableau_t* tab, const size_t targ);
void tableau_SHY(tableau_t* tab, const size_t targ);
void tableau_RHY(tableau_t* tab, const size_t targ);
void tableau_HSH(tableau_t* tab, const size_t targ);
void tableau_HRH(tableau_t* tab, const size_t targ);
void tableau_RHS(tableau_t* tab, const size_t targ);
void tableau_SHR(tableau_t* tab, const size_t targ);

#endif
