#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "instructions.h"
#include "widget.h"

#include <assert.h>

#define _I_ (0x00 | LOCAL_CLIFFORD_MASK)
#define _X_ (0x01 | LOCAL_CLIFFORD_MASK)
#define _Y_ (0x02 | LOCAL_CLIFFORD_MASK)
#define _Z_ (0x03 | LOCAL_CLIFFORD_MASK)
#define _H_ (0x04 | LOCAL_CLIFFORD_MASK)
#define _S_ (0x05 | LOCAL_CLIFFORD_MASK)
#define _R_ (0x06 | LOCAL_CLIFFORD_MASK)
#define _HX_ (0x07 | LOCAL_CLIFFORD_MASK)
#define _SX_ (0x08 | LOCAL_CLIFFORD_MASK)
#define _RX_ (0x09 | LOCAL_CLIFFORD_MASK)
#define _HY_ (0x0a | LOCAL_CLIFFORD_MASK)
#define _HZ_ (0x0b | LOCAL_CLIFFORD_MASK)
#define _SH_ (0x0c | LOCAL_CLIFFORD_MASK)
#define _RH_ (0x0d | LOCAL_CLIFFORD_MASK)
#define _HS_ (0x0e | LOCAL_CLIFFORD_MASK)
#define _HR_ (0x0f | LOCAL_CLIFFORD_MASK)
#define _HSX_ (0x10 | LOCAL_CLIFFORD_MASK)
#define _HRX_ (0x11 | LOCAL_CLIFFORD_MASK)
#define _SHY_ (0x12 | LOCAL_CLIFFORD_MASK)
#define _RHY_ (0x13 | LOCAL_CLIFFORD_MASK)
#define _HSH_ (0x14 | LOCAL_CLIFFORD_MASK)
#define _HRH_ (0x15 | LOCAL_CLIFFORD_MASK)
#define _RHS_ (0x16 | LOCAL_CLIFFORD_MASK)
#define _SHR_ (0x17 | LOCAL_CLIFFORD_MASK)

void test_single_qubit_compositions()
{
    widget_t* wid = widget_create(1); 
    //local_clifford(); 
    return;
}

int main()
{
    test_single_qubit_compositions();
    return 0;
}
