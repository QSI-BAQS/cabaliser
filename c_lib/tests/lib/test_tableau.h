#ifndef TEST_TABLEAU_H
#define TEST_TABLEAU_H

#include "tableau.h"
#include <stdlib.h>

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

void test_tableau_print(uint64_t** arr_a, const size_t n_channels, const size_t x_offset, const size_t y_offset_bytes, const size_t y_offset_bits)
{
    for (size_t i = 0; i < n_channels; i++)
    {
        printf("|");
        for (size_t j = 0; j < n_channels; j++)
        {
            printf("%d ", !!((arr_a[j + x_offset][y_offset_bytes] >> y_offset_bits) & (1 << i)));
        }
        printf("|\n");
    } 
    printf("\n");
}

#endif

