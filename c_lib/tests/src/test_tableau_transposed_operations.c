#include <string.h>

#include "tableau.h"
#include "test_tableau.h"

void test_transposed_hadamard(size_t n_qubits)
{
    tableau_t* tab = tableau_random_create(n_qubits); 
    tableau_t* tab_cmp = tableau_copy(tab);  

    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        tableau_H(tab, i);

        tableau_transpose(tab);

        tableau_transverse_hadamard(tab, i); 

        tableau_transpose(tab);

        assert(*(uint8_t*)&(tab_cmp->slices_x[i][0]) == *(uint8_t*)&(tab->slices_x[i][0]));  
        assert(*(uint8_t*)&(tab_cmp->slices_z[i][0]) == *(uint8_t*)&(tab->slices_z[i][0]));  
        assert(*(uint8_t*)&(tab_cmp->phases[0]) == *(uint8_t*)&(tab->phases[0]));  
    }
}

int main()
{
    test_transposed_hadamard(8);
    return 0;
}
