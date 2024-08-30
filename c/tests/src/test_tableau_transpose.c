#include <string.h>

#include "tableau.h"

tableau_t* tableau_random_create(size_t n_qubits)
{
    tableau_t* tab = tableau_create(n_qubits);
    for (size_t i = 0; i < n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j++)
        {
           tab->slices_x[i][j] = rand(); 
           tab->slices_z[i][j] = rand(); 
        }
    }     
    tab->phases[0] = rand(); 

    return tab;
} 

tableau_t* tableau_copy(tableau_t* tab)
{
    tableau_t* tab_cpy = tableau_random_create(tab->n_qubits); 
    #pragma omp parallel for
    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j++)
        {
            tab_cpy->slices_x[i][j] = tab->slices_x[i][j]; 
            tab_cpy->slices_z[i][j] = tab->slices_z[i][j]; 
        }
    }   
    for (size_t j = 0; j < tab->slice_len; j++)
    {
        tab_cpy->phases[j] = tab->phases[j]; 
    }
    return tab_cpy;
}


void test_small(const size_t n_qubits)
{
    tableau_t* tab = tableau_random_create(n_qubits); 
    tableau_t* tab_cmp = tableau_copy(tab);  

    tableau_transpose(tab);  
    tableau_transpose_naive(tab_cmp);

    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        for (size_t j = 0; j < tab->slice_len; j++)
        {
            assert(tab->slices_x[i][j] == tab_cmp->slices_x[i][j]);
            //assert(tab->slices_z[i][j] == tab_cmp->slices_z[i][j]);
        }
    } 
 
    return;
}


int main()
{

    test_small(64);

//    for (size_t i = 3; i < 256; i++)
//    {
//        test_small(i);
//    }

    return 0;
}
