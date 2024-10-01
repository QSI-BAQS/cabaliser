import cjab.py 

# Rz tags
_T_ = 1
_Tdag_ = 2

toffoli_gates = [
    (api._H_, (2,)),
    (api._CNOT_, (1, 2)),
    (api._RZ_, (2, _Tdag_)),
    (api._CNOT_, (0, 2)),
    (api._RZ_, (2, _T_)),
    (api._CNOT_, (1, 2)),
    (api._RZ_, (2, _Tdag_)),
    (api._CNOT_, (1, 2)),
    (api._RZ_, (2, _T_)),
    (api._CNOT_, (0, 1)),
    (api._RZ_, (0, _T_)),
    (api._RZ_, (1, _Tdag_)),
    (api._CNOT_, (0, 1)),
    (api._H_, (2,))
]

ops = OperationSequence(14)
for opcode, args in tofoli_gates:
    ops.append(opcode, *args)

wid = Widget(3, 20)

wid(ops)

wid.decompose()


