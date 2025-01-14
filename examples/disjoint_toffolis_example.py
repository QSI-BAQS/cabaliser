'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget

# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

n_qubits = 6
max_qubits = 30


# Parameterised toffoli gate
def toffoli_gate(ctrl_a, ctrl_b, targ):
    return [
    (gates.H, (targ,)),
    (gates.CNOT, (ctrl_b, targ)),
    (gates.RZ, (targ, _Tdag_)),
    (gates.CNOT, (ctrl_a, targ)),
    (gates.RZ, (targ, _T_)),
    (gates.CNOT, (ctrl_b, targ)),
    (gates.RZ, (targ, _Tdag_)),
    (gates.CNOT, (ctrl_b, targ)),
    (gates.RZ, (targ, _T_)),
    (gates.RZ, (ctrl_b, _T_)),
    (gates.CNOT, (ctrl_a, ctrl_b)),
    (gates.RZ, (ctrl_a, _T_)),
    (gates.RZ, (ctrl_b, _Tdag_)),
    (gates.CNOT, (ctrl_a, ctrl_b)),
    (gates.H, (targ,))
]

# Create operation sequence
toffoli = toffoli_gate(0, 1, 2)
toffoli_2 = toffoli_gate(3, 4, 5)
ops = OperationSequence(len(toffoli))
ops_2 = OperationSequence(len(toffoli_2))
for opcode, args in toffoli:
    ops.append(opcode, *args)

for opcode, args in toffoli_2:
    ops_2.append(opcode, *args)


# Create widget, do not teleport input
wid = Widget(n_qubits, max_qubits)#, teleport_input=False)

# Apply operation on widget
wid(ops)
wid(ops_2)



# Decompose widget
wid.decompose()

print(wid.json())
