#include <string.h>

#include "tableau.h"
#include "test_tableau.h"


/*
 * test_tableau_transpose
 */
void test_tableau_transpose(const size_t n_qubits)
{


    tableau_t* tab = tableau_random_create(n_qubits); 

    tableau_t* tab_cmp = tableau_copy(tab);  


    // Confirm initial slices
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        for (size_t j = 0; j < (tab->slice_len / CHUNK_SIZE); j++)
         {
            assert(tab->slices_x[i][j] == tab_cmp->slices_x[i][j]);
            assert(tab->slices_z[i][j] == tab_cmp->slices_z[i][j]);
        }
    } 

    tableau_transpose(tab);  

    tableau_transpose_naive(tab_cmp);


    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len / CHUNK_SIZE; j++)
         {
            if (tab->slices_x[i][j] != tab_cmp->slices_x[i][j])
            {
               printf("\tFailed: %lu %lu\n", i, j); 
               j += tab->slice_len / CHUNK_SIZE; 
               i += 64;
            }

            assert(tab->slices_x[i][j] == tab_cmp->slices_x[i][j]);
            assert(tab->slices_z[i][j] == tab_cmp->slices_z[i][j]);
        }
    } 
    return;
}


int main()
{
    test_tableau_transpose(8);

    // Single inplace transpose
    for (size_t i = 0; i < 1000; i++)
    {
        srand(i);
        test_tableau_transpose(64);
    }

    // Aligned transposes 
    for (size_t i = 1; i < 100; i+=10)
    {
        srand(i);
        test_tableau_transpose(64 * i);
    }


    // Unaligned numbers of qubits
    for (size_t i = 1; i < 63; i++)
    {
        srand(i);
        test_tableau_transpose(64 + i);
    }

    // Unaligned numbers of qubits with multiple chunk transposes
    for (size_t i = 1; i < 63; i++)
    {
        srand(i);
        test_tableau_transpose(64 * i + i);
    }

    return 0;
}
