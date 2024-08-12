#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>



#include "instructions.h"


#include <assert.h>

extern instruction_t local_clifford(const instruction_t left, const instruction_t right);

void test_single_qubit_compositions()
{
    // Some basic and self-evident tests
    assert(LOCAL_CLIFFORD(_I_, _I_) == _I_);
    assert(LOCAL_CLIFFORD(_X_, _I_) == _X_);
    assert(LOCAL_CLIFFORD(_X_, _X_) == _I_);
    assert(LOCAL_CLIFFORD(_H_, _H_) == _I_);
    assert(LOCAL_CLIFFORD(_X_, _Z_) == _Y_);

    // Demonstrate non-commutivity
    assert(LOCAL_CLIFFORD(_S_, _HR_) == _SHR_);
    return;
}


void test_identity(void)
{
    for (size_t i = 0; i < N_LOCAL_CLIFFORD_INSTRUCTIONS; i++)
    {
        uint8_t operator = i | LOCAL_CLIFFORD_MASK; 
        assert(LOCAL_CLIFFORD(operator, _I_) == operator);
        assert(LOCAL_CLIFFORD(_I_, operator) == operator);
    }
    return;
}


void test_self_inverse(void)
{
    assert(LOCAL_CLIFFORD(_I_, _I_) == _I_);
    assert(LOCAL_CLIFFORD(_X_, _X_) == _I_);
    assert(LOCAL_CLIFFORD(_Y_, _Y_) == _I_);
    assert(LOCAL_CLIFFORD(_Z_, _Z_) == _I_);
    assert(LOCAL_CLIFFORD(_H_, _H_) == _I_);
    assert(LOCAL_CLIFFORD(_S_, _R_) == _I_);
    assert(LOCAL_CLIFFORD(_R_, _S_) == _I_);
    assert(LOCAL_CLIFFORD(_H_, _HX_) == _X_);
    assert(LOCAL_CLIFFORD(_R_, _SX_) == _X_);
    assert(LOCAL_CLIFFORD(_S_, _RX_) == _X_);
    assert(LOCAL_CLIFFORD(_H_, _HY_) == _Y_);
    assert(LOCAL_CLIFFORD(_H_, _HZ_) == _Z_);
    assert(LOCAL_CLIFFORD(_R_, _SH_) == _H_);
    assert(LOCAL_CLIFFORD(_S_, _RH_) == _H_);
    assert(LOCAL_CLIFFORD(_H_, _HS_) == _S_);
    assert(LOCAL_CLIFFORD(_H_, _HR_) == _R_);
    assert(LOCAL_CLIFFORD(_H_, _HSX_) == _SX_);
    assert(LOCAL_CLIFFORD(_H_, _HRX_) == _RX_);
    assert(LOCAL_CLIFFORD(_R_, _SHY_) == _HY_);
    assert(LOCAL_CLIFFORD(_S_, _RHY_) == _HY_);
    assert(LOCAL_CLIFFORD(_H_, _HSH_) == _SH_);
    assert(LOCAL_CLIFFORD(_H_, _HRH_) == _RH_);
    assert(LOCAL_CLIFFORD(_S_, _RHS_) == _HS_);
    assert(LOCAL_CLIFFORD(_R_, _SHR_) == _HR_);
}

int main()
{
    test_single_qubit_compositions();
    test_self_inverse();

    return 0;
}
