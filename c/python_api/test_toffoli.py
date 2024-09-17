import cjab 
import gates 


# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

n_qubits = 3
max_qubits = 20


# Parameterised toffoli gate
toffoli_gate = lambda ctrl_a, ctrl_b, targ: [
    (gates._H_, (targ,)),
    (gates._CNOT_, (ctrl_b, targ)),
    (gates._RZ_, (targ, _Tdag_)),
    (gates._CNOT_, (ctrl_a, targ)),
    (gates._RZ_, (targ, _T_)),
    (gates._CNOT_, (ctrl_b, targ)),
    (gates._RZ_, (targ, _Tdag_)),
    (gates._CNOT_, (ctrl_b, targ)),
    (gates._RZ_, (targ, _T_)),
    (gates._CNOT_, (ctrl_a, ctrl_b)),
    (gates._RZ_, (ctrl_a, _T_)),
    (gates._RZ_, (ctrl_b, _Tdag_)),
    (gates._CNOT_, (ctrl_a, ctrl_b)),
    (gates._H_, (targ,))
]

# Create operation sequence
toffoli = toffoli_gate(0, 1, 2)
ops = cjab.OperationSequence(len(toffoli))
for opcode, args in toffoli:
    ops.append(opcode, *args)

# Create widget, do not teleport input
wid = cjab.Widget(n_qubits, max_qubits, teleport_input=False)

# Apply operation on widget
wid(ops)

# Decompose widget
wid.decompose()

# Test Adjacencies
for i in (0, 1, 7, 8): 
    assert(0 == wid.get_adjacencies(i).n_adjacent)
assert(4 == wid.get_adjacencies(2).n_adjacent)
for i in range(3, 7):
    assert(1 == wid.get_adjacencies(i).n_adjacent)

# Test Measurement tags
measurements = [_T_, _Tdag_, _Tdag_, _T_, _Tdag_, _T_, _I_, _I_, _I_]
for m, tag in zip(measurements, wid.get_measurement_tags()): 
    assert(m == tag)

# Test IO
measurements = [_T_, _Tdag_, _Tdag_, _T_, _Tdag_, _T_, _I_, _I_, _I_]
for m, tag in zip(measurements, wid.get_measurement_tags()): 
    assert(m == tag)

io_map = [7, 8, 6]
for i, j in zip(io_map, wid.get_io_map()):
    assert(i == j)
