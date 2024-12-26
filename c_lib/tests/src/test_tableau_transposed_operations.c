#include <string.h>

#include "tableau.h"
#include "test_tableau.h"

void test_transposed_hadamard(size_t n_qubits)
{
    tableau_t* tab = tableau_random_create(n_qubits); 
    tableau_t* tab_cmp = tableau_copy(tab);  
    const size_t slice_len = tab->slice_len;


    for (size_t i = 0; i < tab->n_qubits; i++)
    {
        tableau_H(tab, i);

        tableau_transpose(tab);

        tableau_transverse_hadamard(tab, i); 

        tableau_transpose(tab);
    

        for (size_t j = 0; (j < slice_len) && (j * 8 < (n_qubits / 8) * 8); j++)
        {
            assert(
                    ((uint8_t*)(tab_cmp->slices_x[i]))[j] 
                    ==
                    ((uint8_t*)(tab->slices_x[i]))[j]
                );

            assert(
                    ((uint8_t*)(tab_cmp->slices_z[i]))[j] 
                    ==
                    ((uint8_t*)(tab->slices_z[i]))[j]
                );

            assert(
                    ((uint8_t*)(tab_cmp->phases))[j] 
                    ==
                    ((uint8_t*)(tab->phases))[j]
                );

        }

        const size_t offset = n_qubits % 8; 
        const uint8_t mask = (1 << offset) - 1;
        const size_t idx = ((n_qubits / 8) * 8) / 8;

        assert(
                (((uint8_t*)(tab_cmp->slices_x[i]))[idx] & mask) 
                ==
                (((uint8_t*)(tab->slices_x[i]))[idx] & mask)
            );

        assert(
                (((uint8_t*)(tab_cmp->slices_z[i]))[idx] & mask) 
                ==
                (((uint8_t*)(tab->slices_z[i]))[idx] & mask)
            );

        assert(
                (((uint8_t*)(tab_cmp->phases))[idx] & mask) 
                ==
                (((uint8_t*)(tab->phases))[idx] & mask)
            );
    }
}

int main()
{
    for (size_t i = 263; i < 1280; i += 17)
    {
        printf("%zu\n", i);
        test_transposed_hadamard(i);
    }
    return 0;
}
