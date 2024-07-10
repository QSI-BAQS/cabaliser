#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "tableau.h"
#include <assert.h>

void test_tableau_ctz(const size_t n_qubits)
{
    tableau_t* tab = tableau_create(n_qubits);

    for (size_t i = 0; i < n_qubits; i++)
    {   
        tableau_ctz(tab->slices_z[i], tab->n_qubits);

        assert(tableau_slice_empty_x(tab, i));
        assert(i == tableau_ctz(tab->slices_z[i], tab->n_qubits)); 
    }
    tableau_destroy(tab);
}



int main()
{
    
    test_tableau_ctz(64);
    test_tableau_ctz(128);
    test_tableau_ctz(256);


    return 0;
};
