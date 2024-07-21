#include <assert.h>

#include "widget.h"
#include "input_stream.h"
#include "instructions.h"

void test_widget_create()
{
    widget_t* wid = widget_create(4, 8); 
    assert(4 == wid->n_qubits); 
    assert(8 == wid->max_qubits); 
    widget_destroy(wid); 
    return;
}

void test_widget_local_clifford()
{


};

int main()
{
    test_widget_create();
    return 0;
}
