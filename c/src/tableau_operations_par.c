#define TABLEAU_OPERATIONS_PAR_SRC
#include "tableau_operations_par.h"

void tableau_par_H(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] & slice_z[i];      
    }  
    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}


void tableau_par_S(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] & slice_z[i];      
        slice_z[i] ^= slice_x[i]; 
    }  
    free(op);
}

void tableau_par_Z(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
/*
 * Doubled S gate
 * S: (r ^= x.z; z ^= x)
 * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
 * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
 * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
 * r2 = r0 ^ x; z2 = z0 
 */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i];      
    }  
    free(op);
}


void tableau_par_R(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * Triple S gate
     * S : (r ^= x.z; z ^= x)
     * r1 = r0 ^ (x.z0); z1 = z0 ^ x 
     *
     * r2 = r0 ^ (x.z0) ^ (x.(z0 ^ x)); z2 = z0 ^ x ^ x 
     * r2 = r0 ^ x.(z0 ^ z0 ^ x); z2 = z0 
     * r2 = r0 ^ x; z2 = z0 
     *
     * r3 = r0 ^ x ^ x.z2; z3 = z2 ^ x 
     * r3 = r0 ^ x ^ x.z0; z3 = z0 ^ x 
     *
     * R : (r ^= x.~z; z ^= x)
     */
    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] & ~slice_z[i];
        slice_z[i] ^= slice_x[i]; 
    }  
    free(op);
}


void tableau_par_I(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    return;
    free(op);
}

void tableau_par_X(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
/*
 * HZH Gate
 * Flip X and Z, perform a Z, then flip X and Z again
 * H : (r ^= x.z; x <-> z) 
 * Z : (r ^= x)
 * 
 * H
 * r_1 = r_0 ^ x0.z0; x1 = z0; z1 = x0  
 * 
 * Z
 * r_2 = r_1 ^ x1; x2 = x1; z2 = z1  
 * r_2 = r_0 ^ x0.z0 ^ z0; x2 = z0; z2 = x0  
 *
 * H
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z2.x2; x3 = z2; z3 = x2  
 * r_3 = r_0 ^ x0.z0 ^ z0 ^ z0.x0; x3 = x0; z3 = z0  
 * r_3 = r_0 ^ z0;
 *
 */
     for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_z[i];      
    }
    free(op);
}

void tableau_par_Y(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * Y = XZ
     * Z : (r ^= x)
     * X : (r ^= z)
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ z 
     *
     * Y : r ^= x ^ z
     */
    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_z[i] ^ slice_x[i];      
    }  
    free(op);
}

void tableau_par_HX(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * Swap x and z
     */
    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= ~slice_x[i] & slice_z[i];      
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}

void tableau_par_SX(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.z 
     * r_2 = r_0 ^ (~x & z) 
     * z_2 = z_0 ^ x
     */

     for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= ~slice_x[i] & slice_z[i];      
        slice_z[i] ^= slice_x[i]; 
    }  
    free(op);
}



void tableau_par_RX(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_0 ^ z ^ x.~z 
     * r_2 = r_0 ^ (z | x) 
     * z_2 = z_0 ^ x
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] | slice_z[i];      
        slice_z[i] ^= slice_x[i]; 
    }  
    free(op);
}

void tableau_par_HZ(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * Z : (r ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x 
     *
     * r_2 = r_0 ^ x ^ x.z 
     * r_2 = r_0 ^ (x & ~z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= ~slice_z[i] & slice_x[i];      
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}



void tableau_par_HY(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 
    /*
     * Y : r ^= x ^ z
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x ^ z 
     *
     * r_2 = r_0 ^ x ^ z ^ x.z 
     * r_2 = r_0 ^ (x | z) 
     * z_2 = x_0
     * x_2 = z_0
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_z[i] ^ slice_x[i];      
    }  
    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}


void tableau_par_SH(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    /*
     * H : (r ^= x.z; x <-> z) 
     * S : (r ^= x.z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.x_0
     * r_2 = r_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     * 
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_x[i] ^= slice_z[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}


void tableau_par_RH(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * H : (r ^= x.z; x <-> z) 
     * R : (r ^= x.~z; z ^= x)
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = x_0
     * x_1 = z_0
     *
     * r_2 = r_1 ^ x_1.~z_1
     * r_2 = r_0 ^ x_0.z_0 ^ z_0.~x_0
     * r_2 = r_0 ^ z_0
     * z_2 ^= x_2 -> x_0 ^= z_0
     *
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}


void tableau_par_HS(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    /*
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x.z 
     * z_1 = z_0 ^ x
     *
     * r_2 = r_1 ^ x.z_1
     * r_2 = r_0 ^ x.z_0 ^ x.(z_0 ^ x)
     * z_2 = x_1
     * x_2 = z_1
     *
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_x[i] ^= slice_z[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}

void tableau_par_HR(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    /*
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ x.~z 
     * z_1 = z_0 ^ x
     *
     * r_2 = r_1 ^ x.~z_1
     * r_2 = r_0 
     *
     * z_2 = x_1
     * x_2 = z_1 = z_0 ^ x_0
     *
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_z[i] ^= slice_x[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}

void tableau_par_HSX(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * S : (r ^= x.z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.z_0
     * r_2 = r_0 ^ z_0 ^ x.z_0 
     * r_2 = r_0 ^ ~x.z_0
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     * r_3 = r_0 ^ x_0 ^ z_0
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] ^ slice_z[i];
        slice_z[i] ^= slice_x[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}

void tableau_par_HRX(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    /*
     * X : (r ^= z)
     * R : (r ^= x.~z; z ^= x)
     * H : (r ^= x.z; x <-> z) 
     *
     * r_1 = r_0 ^ z 
     *
     * r_2 = r_1 ^ x.~z_0
     * r_2 = r_0 ^ z_0 ^ x.~z_0 
     * r_2 = r_0 ^ (x | z_0)
     * z_2 = z_0 ^ x 
     *
     * r_3 = r_2 ^ x_0.z_2 
     * r_3 = r_0 ^ ~x_0.z_0 ^ x_0.(z_0 ^ x_0)
     * r_3 = r_0 ^ z
     * x_3 = z_2 = x_0 ^ z_0 
     * z_3 = x_2 = x_0
     */

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_z[i];
        slice_z[i] ^= slice_x[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}

void tableau_par_SHY(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_z[i] ^ slice_x[i];
        slice_x[i] ^= slice_z[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}

void tableau_par_RHY(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i];
        slice_x[i] ^= slice_z[i];
    }  

    void* ptr = tab->slices_x[op->targ];
    tab->slices_x[op->targ] = tab->slices_z[op->targ];
    tab->slices_z[op->targ] = ptr;
    free(op);
}

void tableau_par_HSH(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= ~slice_x[i] & slice_z[i];
        slice_x[i] ^= slice_z[i];
    }
    free(op);
}

void tableau_par_HRH(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] & slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  
    free(op);
}


void tableau_par_RHS(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] | slice_z[i];
        slice_x[i] ^= slice_z[i];
    }
    free(op);
}


void tableau_par_SHR(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    CHUNK_OBJ* slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= slice_x[i] & ~slice_z[i];
        slice_x[i] ^= slice_z[i];
    }  
    free(op);
}

void tableau_par_CNOT(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    /*
     * CNOT a, b: ( 
     *     r ^= x_a & z_b & (1 ^ x_b ^ z_a);
     *     x_b ^= x_a;
     *     z_a ^= z_b) 
     *
     */ 

    CHUNK_OBJ* ctrl_slice_x = (CHUNK_OBJ*)(tab->slices_x[op->ctrl]); 
    CHUNK_OBJ* ctrl_slice_z = (CHUNK_OBJ*)(tab->slices_z[op->ctrl]); 
    CHUNK_OBJ* targ_slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* targ_slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= ctrl_slice_x[i] & targ_slice_z[i] & ~(targ_slice_x[i] ^ ctrl_slice_z[i]);
        targ_slice_x[i] ^= ctrl_slice_x[i];
        ctrl_slice_z[i] ^= targ_slice_z[i];
    }  
    free(op);
}

void tableau_par_CZ(struct distributed_tableau_op* op)
{
    tableau_t* tab = op->tab;

    /* CZ = H_b CNOT H_b 
     * H : (r ^= x.z; x <-> z) 
     * CNOT a, b: ( 
     *     r ^= x_a & z_b & ~(x_b ^ z_a);
     *     x_b ^= x_a;
     *     z_a ^= z_b) 
     *
     * H:
     * r_1 = r_0 ^ x_b.z_b 
     * x_a1 = x_a0
     * x_b1 = z_b0 
     * z_a1 = z_a0
     * z_b1 = x_b0 
     *
     * CNOT:
     * r_2 = r_1 ^ (x_a0 & z_b1 & ~(x_b1 ^ z_a0))
     * r_2 = r_0 ^ (x_b0 & z.b0) ^ (x_a0 & x_b0 & ~(z_b0 ^ z_a0))
     * x_a2 = x_a1
     * x_a2 = x_a0 
     * x_b2 = x_b1 ^ x_a1
     * x_b2 = z_b0 ^ x_a0
     * z_a2 = z_a1 ^ z_b1 
     * z_a2 = z_a0 ^ x_b0 
     * z_b2 = z_b1
     * z_b2 = x_b0
     *
     * H: 
     * r_3 = r_2 ^ (x_b2 & z_b2)  
     * r_3 = r_0 ^ (x_b0 & z_b0) ^ (x_a0 & x_b0 & ~(z_b0 ^ z_a0)) ^ ((z_b0 ^ x_a0) & x_b0); 
     * x_a3 = x_a2 = x_a0 
     * x_b3 = z_b2 = x_b0 
     * z_a3 = z_a2 = z_a0 ^ x_b0 
     * z_b3 = x_b2 = z_b0 ^ x_a0 
     *
     */ 

    CHUNK_OBJ* ctrl_slice_x = (CHUNK_OBJ*)(tab->slices_x[op->ctrl]); 
    CHUNK_OBJ* ctrl_slice_z = (CHUNK_OBJ*)(tab->slices_z[op->ctrl]); 
    CHUNK_OBJ* targ_slice_x = (CHUNK_OBJ*)(tab->slices_x[op->targ]); 
    CHUNK_OBJ* targ_slice_z = (CHUNK_OBJ*)(tab->slices_z[op->targ]); 
    CHUNK_OBJ* slice_r = (CHUNK_OBJ*)(tab->phases); 

    for (size_t i = op->start; i < op->stop; i++)
    {
        slice_r[i] ^= (
              (targ_slice_x[i] & targ_slice_z[i]) 
            ^ (ctrl_slice_x[i] & targ_slice_x[i] & ~(targ_slice_z[i] ^ ctrl_slice_z[i]))
            ^ ((targ_slice_z[i] ^ ctrl_slice_x[i]) & targ_slice_x[i])
            );
        targ_slice_z[i] ^= ctrl_slice_x[i];
        ctrl_slice_z[i] ^= targ_slice_x[i];
    }  
    free(op);
}
