import cjab 
import gates 


# Rz tags
_T_ = 1
_Tdag_ = 2

toffoli_gates = [
    (gates._H_, (2,)),
    (gates._CNOT_, (1, 2)),
    (gates._RZ_, (2, _Tdag_)),
    (gates._CNOT_, (0, 2)),
    (gates._RZ_, (2, _T_)),
    (gates._CNOT_, (1, 2)),
    (gates._RZ_, (2, _Tdag_)),
    (gates._CNOT_, (1, 2)),
    (gates._RZ_, (2, _T_)),
    (gates._CNOT_, (0, 1)),
    (gates._RZ_, (0, _T_)),
    (gates._RZ_, (1, _Tdag_)),
    (gates._CNOT_, (0, 1)),
    (gates._H_, (2,))
]

ops = cjab.OperationSequence(14)
for opcode, args in toffoli_gates:
    ops.append(opcode, *args)

wid = cjab.Widget(3, 20)

wid(ops)

wid.decompose()

for i in (0, 1, 7, 8): 
    assert(0 == wid.get_adjacencies(i).n_adjacent)

assert(4 == wid.get_adjacencies(2).n_adjacent)

for i in range(3, 7):
    assert(1 == wid.get_adjacencies(i).n_adjacent)
