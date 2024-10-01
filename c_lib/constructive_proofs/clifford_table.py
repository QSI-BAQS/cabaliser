import scipy
import numpy as np


I = np.eye(2)
X = np.array([[0, 1],
      [1, 0]])
Y = np.array([[ 0.+0.j, -0.-1.j],
      [ 0.+1.j,  0.+0.j]])
Z = np.array([[ 1,  0],
      [ 0, -1]])
H = 2 ** -0.5 * np.array([[ 1,  1],
      [ 1, -1]])
S = np.array([[ 1,  0],
      [ 0, 1j]])
R = np.array([[ 1,  0],
      [ 0, -1j]])



def kr(*args):
     return reduce(np.kron, filter(lambda x: len(x) > 0, args))


def cmp_gates(gate_a, gate_b, eps=1e-5):
    '''
        Testing gate equivalence
    '''
    tol = int(-1 * np.log10(eps))
    n_qubits = int(np.log2(gate_a.shape[0]))
    
    consts = None
    for basis in range(1 << n_qubits):
        basis_state = np.zeros(1 << n_qubits, dtype=np.complex128)  
        basis_state[basis] = 1 

        state = np.round(gate_a.transpose().conj() @ gate_b @ basis_state, tol) 
        if sum(map(lambda x: (x > eps), state)) > 1: 
            return False
        if consts is None:     
            consts = state[basis] 
        if any(map(lambda x: np.abs(x[0] - consts * x[1]) > eps, zip(state, basis_state))):
            return False
    return True


SINGLE_QUBIT_CLIFFORDS = {
    'I':I,
    'X':X,
    'Y':Y,
    'Z':Z,
    'H':H,
    'S':S,
    'R':R
}

cliffords = dict(SINGLE_QUBIT_CLIFFORDS) 
appended_gates = {'I':I}
last_generation = dict(SINGLE_QUBIT_CLIFFORDS) 

# Greedily construct cliffords
while len(appended_gates) > 0:
    appended_gates = {}
    for gate_str, gate in last_generation.items():
        for cliff_str, cliff_gate in SINGLE_QUBIT_CLIFFORDS.items(): 
            if len(gate_str) > 0 and cliff_str == gate_str[0]:
                continue

            target_gate = cliff_gate @ gate
            for cmp_gate in cliffords.values():
                if cmp_gates(cmp_gate, target_gate):
                    break 
            else:
                cliffords[cliff_str + gate_str] = target_gate
                appended_gates[cliff_str + gate_str] = target_gate
    last_generation = appended_gates  


# Map of all single qubit clifford operations
# These operations are applied to the left of the clifford as a matrix operation
clifford_map_left = {}
for gate_str, gate in cliffords.items():
    clifford_map_left[gate_str] = {}
    for cliff_str, cliff_gate in SINGLE_QUBIT_CLIFFORDS.items(): 
        if len(gate_str) > 1 and cliff_str == gate_str[0]:
            clifford_map_left[gate_str][cliff_str] = gate_str[1:]
        target_gate = cliff_gate @ gate;
        
        for cmp_str, cmp_gate in cliffords.items():
            if cmp_gates(cmp_gate, target_gate):
                clifford_map_left[gate_str][cliff_str] = cmp_str
                break  

# Map of all single qubit clifford operations
# These operations are applied to the right of the clifford as a matrix operation
clifford_map_right = {}
for gate_str, gate in cliffords.items():
    clifford_map_right[gate_str] = {}
    for cliff_str, cliff_gate in SINGLE_QUBIT_CLIFFORDS.items(): 
        if len(gate_str) > 1 and cliff_str == gate_str[0]:
            clifford_map_right[gate_str][cliff_str] = gate_str[1:]
        target_gate = gate @ cliff_gate;
        
        for cmp_str, cmp_gate in cliffords.items():
            if cmp_gates(target_gate, cmp_gate):
                clifford_map_right[gate_str][cliff_str] = cmp_str
                break  



# Integer indexing to avoid dictionary order issues
clifford_list = {i:j for i, j in enumerate(cliffords)} 
byte_map = {gate_str: hex(i) for i, gate_str in enumerate(cliffords)}
rev_byte_map = [[int(j, 16), i] for i, j in byte_map.items()] 
rev_byte_map.sort()


def single_qubit_lookup_table(clifford_map):
    single_cliff_byte_map = [[int(byte_map[i], 16), i] for i in SINGLE_QUBIT_CLIFFORDS]     

    single_cliff_byte_map.sort()

    single_qubit_table = {}
    for i in map(lambda x: x[1], single_cliff_byte_map):
        single_qubit_table[i] = []
        for j in clifford_map:
            gate_str = clifford_map[j][i]
            single_qubit_table[i].append(clifford_map[j][i])
    for i in single_qubit_table:  
        print(f"/* {i} */", end='')
        for j in single_qubit_table[i]: 
            print(f" _{j}_", end=',')
        print()


