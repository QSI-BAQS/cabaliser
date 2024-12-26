#include <stdlib.h>
#include <string.h>

#include "tableau.h"


/*
 * Creates a random bitmatrix
 * This is not guaranteed to correspond to a stabiliser matrix
 * :: n_qubits : size_t :: Number of qubits corresponding to number of rows and columns
 */
tableau_t* tableau_random_create(size_t n_qubits)
{
    tableau_t* tab = tableau_create(n_qubits);
    for (size_t i = 0; i < n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j++)
        {
           ((uint8_t*)(tab->slices_x[i]))[j] = rand(); 
           ((uint8_t*)(tab->slices_z[i]))[j] = rand(); 
        }
    }     
    tab->phases[0] = rand(); 

    return tab;
}


/*
 * tableau_copy
 * Creates a copy of an existing tableau object
 * This is useful for testing the action of operations on the tableau
 * :: tab : tableau_t* :: The tableau to copy
 */
tableau_t* tableau_copy(tableau_t* const tab)
{
    tableau_t* tab_cpy = tableau_create(tab->n_qubits); 
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        memcpy(tab_cpy->slices_x[i], tab->slices_x[i], tab->slice_len);
        memcpy(tab_cpy->slices_z[i], tab->slices_z[i], tab->slice_len);
    }   
    for (size_t j = 0; j < tab->slice_len / CHUNK_SIZE; j++)
    {
        memcpy(tab_cpy->phases, tab->phases, tab->slice_len);
    }
    return tab_cpy;
}


void benchmark_transposed_hadamard(size_t n_qubits)
{
    tableau_t* tab = tableau_random_create(n_qubits); 
    tableau_t* tab_cmp = tableau_copy(tab);  
    const size_t slice_len = tab->slice_len;

    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        tableau_transverse_hadamard(tab, i); 
    }
    tableau_destroy(tab);
    tableau_destroy(tab_cmp);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Insufficient parameters, requires <n_qubits> <seed>\n");
    return 0;
    }

    size_t tableau_size = atoi(argv[1]);
    uint32_t seed = atoi(argv[2]);
     
    srand(seed);

    benchmark_transposed_hadamard(tableau_size);

    return 0;
}
