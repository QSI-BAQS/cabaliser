#include "tableau.h"
#include <assert.h>


void test_tableau_create(const size_t n_qubits)
{
    tableau_t* tab = tableau_create(n_qubits);
    assert(NULL != tab);

    for (size_t i = 0; i < n_qubits; i++)
    {   

        assert(tableau_slice_empty_x(tab, i));

//        void* slice_z = tab->slices_z[i]; 
//        
//
//        for (size_t j = 0; j < n_qubits; j++)
//        {   
//            // Initial tableau should have
//            // 1 along the Z diagonal
//            if (i == j)
//            {
//                assert(1 == slice_get_bit(slice_z, j));
//            }
//            else // And 0 elsewhere
//            {   
//               assert(0 == slice_get_bit(slice_z, j));
//            }
//        }
    }
    tableau_destroy(tab);
}


void test_hadamard(const size_t n_qubits)
{
    tableau_t* tab = tableau_create(n_qubits);

    assert(NULL != tab);

//    for (size_t targ = 0; targ < n_qubits; targ++)
//        for (size_t i = 0; i < n_qubits; i++)
//        {   
//            void* slice_z = tab->slices_z[i]; 
//            void* slice_x = tab->slices_x[i]; 
//
//            if (i == targ)
//            {
//                assert(1 == slice_get_bit(slice_x, j));
//            }
//
//
//            for (size_t j = 0; j < n_qubits; j++)
//            {   
//                // Initial tableau should have
//                // 1 along the Z diagonal
//                if (i == targ)
//                {
//                    assert(1 == slice_get_bit(slice_x, j));
//                }
//                else // And 0 elsewhere
//                {   
//                   assert(0 == slice_get_bit(slice_z, j));
//                   assert(0 == slice_get_bit(slice_x, j));
//                }
//            }
//        }
//    }

    tableau_destroy(tab);
}


int main()
{
    // Test for small tableaus
    // These all fit in a single cache line chunk 
    for (size_t n_qubits = 0;
         n_qubits < 256;
         n_qubits++)
    {
        test_tableau_create(n_qubits);
    }


    // Test at max chunk size, then increase that
    // These all work over slices that are multiple chunks
    for (size_t n_qubits = CACHE_SIZE_BITS;
         n_qubits < CACHE_SIZE_BITS * 50 + 1;
         n_qubits += CACHE_SIZE_BITS)
    {   
        test_tableau_create(n_qubits);
    }

    return 0;    
}
