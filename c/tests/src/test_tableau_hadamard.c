#include "tableau.h"
#include <assert.h>

void test_hadamard(const size_t n_qubits)
{
    // Initial structure of the tableau is confirmed by the tableau create tests 
    tableau_t* tab = tableau_create(n_qubits);
    assert(NULL != tab);


    for (size_t i = 0; i < n_qubits; i++)
    {   
        assert(1 == slice_get_bit(tab->slices_z[i], i));        
        assert(0 == slice_get_bit(tab->slices_x[i], i));        

        tableau_hadamard(tab, i);

        assert(0 == slice_get_bit(tab->slices_z[i], i));        
        assert(1 == slice_get_bit(tab->slices_x[i], i));        

        tableau_hadamard(tab, i);

        assert(1 == slice_get_bit(tab->slices_z[i], i));        
        assert(0 == slice_get_bit(tab->slices_x[i], i));        

    }

    tableau_destroy(tab);
}

void test_inline_hadamard(const size_t n_qubits)
{
    // Initial structure of the tableau is confirmed by the tableau create tests 
    tableau_t* tab = tableau_create(n_qubits);
    assert(NULL != tab);


    for (size_t i = 0; i < n_qubits; i++)
    {   
        assert(1 == __inline_slice_get_bit(tab->slices_z[i], i));        
        assert(0 == __inline_slice_get_bit(tab->slices_x[i], i));        

        __inline_tableau_hadamard(tab, i);

        assert(0 == __inline_slice_get_bit(tab->slices_z[i], i));        
        assert(1 == __inline_slice_get_bit(tab->slices_x[i], i));        

        __inline_tableau_hadamard(tab, i);

        assert(1 == __inline_slice_get_bit(tab->slices_z[i], i));        
        assert(0 == __inline_slice_get_bit(tab->slices_x[i], i));        

    }

    tableau_destroy(tab);
}


void test_transverse_hadamard()
{
    tableau_t* tab = tableau_create(8);
    for (int i = 0; i < (1 << 8); i++)
    {
        *(tab->slices_z[i]) = i;
    }

    for (int i = 0; i < (1 << 8); i++)
    {
        tableau_hadamard(tab, i); 
    }

    for (int i = 0; i < (1 << 8); i++)
    {
        assert(*tab->slices_z[i] == 0);
        assert(*tab->slices_x[i] == i);
    }


    tableau_transpose(tab);
    
    for (int i = 0; i < (1 << 8); i++)
    {
        tableau_transverse_hadamard(tab, i); 
    }

    tableau_transpose(tab);

    for (int i = 0; i < (1 << 8); i++)
    {
        assert(*tab->slices_z[i] == i);
        assert(*tab->slices_x[i] == 0);
    }


    return;
}


int main()
{

    for (size_t n_qubits = CACHE_SIZE_BITS;
         n_qubits < CACHE_SIZE_BITS * 20 + 1;
         n_qubits += CACHE_SIZE_BITS)
    {   
        test_hadamard(n_qubits);
        test_inline_hadamard(n_qubits);

    }

};
