#include "tableau.h"
#include <assert.h>

void test_tableau_get_bit(const size_t n_qubits)
{
    uint8_t* arr = malloc(n_qubits / 8 + 1); 
    memset(arr, 0, n_qubits / 8 + 1);

    for (size_t i = 0; i < n_qubits; i++)
    {   
        assert(0 == slice_get_bit((void*)arr, i));
        arr[i / 8] ^= 1 << (i % 8);  
        assert(1 == slice_get_bit((void*)arr, i));
        arr[i / 8] ^= 1 << (i % 8);  
        assert(0 == slice_get_bit((void*)arr, i));
    }
    free(arr);
}

void test_tableau_set_bit(const size_t n_qubits)
{
    uint8_t* arr = malloc(n_qubits / 8 + 1); 
    memset(arr, 0, n_qubits / 8 + 1);

    for (size_t i = 0; i < n_qubits; i++)
    {   
        slice_set_bit((void*)arr, i, 1);
        assert(1 == slice_get_bit((void*)arr, i));

        arr[i / 8] ^= 1 << (i % 8);  
        assert(0 == slice_get_bit((void*)arr, i));

        arr[i / 8] ^= 1 << (i % 8);  
        slice_set_bit((void*)arr, i, 0);
        assert(0 == slice_get_bit((void*)arr, i));
    }
    free(arr);
}



void test_slice_len(const size_t n_qubits)
{

    assert(n_qubits / 8 <= (SLICE_LEN_BYTES(n_qubits, sizeof(uint8_t))));
    assert(n_qubits / 8 <= (SLICE_LEN(n_qubits, sizeof(uint8_t))));
    assert(n_qubits / 8 + 1 >= (SLICE_LEN(n_qubits, sizeof(uint8_t))));



    assert(n_qubits / 64 <= (SLICE_LEN(n_qubits, sizeof(size_t))));
    assert(n_qubits / 64 + 8 >= (SLICE_LEN(n_qubits, sizeof(size_t))));


    //SLICE_LEN_BYTES(n_qubits, CACHE_SIZE);
    //SLICE_LEN_BYTES(n_qubits, sizeof(size_t));


}


void test_set_n_qubits(const size_t initial, const size_t final)
{

    tableau_t* tab = tableau_create(initial);

    assert(tab->n_qubits == initial);
    assert(tab->slice_len >= initial / 8);
    assert(tab->slice_len <= initial / 8 + CACHE_SIZE);

    tableau_set_n_qubits(tab, final);
    assert(tab->n_qubits == final);
    assert(tab->slice_len >= final / 8);
    assert(tab->slice_len <= final / 8 + CACHE_SIZE);

    tableau_destroy(tab);
}


int main()
{
    for (size_t n_qubits = 1;
         n_qubits < 256 * 256;
         n_qubits += 253)
    {
        test_tableau_get_bit(n_qubits);
    }

    for (size_t n_qubits = 1;
         n_qubits < 256 * 256;
         n_qubits += 253)
    {
        test_tableau_set_bit(n_qubits);
    }

    for (size_t n_qubits = 253;
         n_qubits < 256 * 256;
         n_qubits += 1023)
    {
        test_slice_len(n_qubits);
    }

    for (size_t n_qubits = 253;
         n_qubits < 256 * 256;
         n_qubits += 1023)
    {
        test_set_n_qubits(n_qubits, n_qubits - 17);
    }
    return 0;    
}
