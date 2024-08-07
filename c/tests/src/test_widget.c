#include <assert.h>
#include <stdlib.h>

#define INSTRUCTIONS_TABLE

#include "widget.h"
#include "input_stream.h"
#include "instructions.h"

#define N_TEST_ITERATIONS (10)
void test_widget_create()
{
    widget_t* wid = widget_create(4, 8); 
    assert(4 == wid->n_qubits); 
    assert(8 == wid->max_qubits); 
    widget_destroy(wid); 
    return;
}

void test_initial_map()
{
    for (size_t j = 0; j < N_TEST_ITERATIONS; j++)
    { 
        const size_t n_qubits = rand() % (1 << 12);
        const size_t max_qubits = n_qubits + rand() % (1 << 12);

        widget_t* wid = widget_create(n_qubits, max_qubits); 
        for (size_t i = 0; i < n_qubits; i++)
        {
            assert(wid->q_map[i] == i);
        } 
        widget_destroy(wid);
    }
}


void test_initial_cliffords()
{
    for (size_t j = 0; j < N_TEST_ITERATIONS; j++)
    {
        const size_t n_qubits = rand() % (1 << 12);
        const size_t max_qubits = n_qubits + rand() % (1 << 12);

        widget_t* wid = widget_create(n_qubits, max_qubits); 
        for (size_t i = 0; i < n_qubits; i++)
        {
            assert(wid->queue->table[i] == _I_);
        } 
        widget_destroy(wid);
    }
}


int main()
{
    test_widget_create();
    test_initial_map();
    test_initial_cliffords();
    return 0;
}
