'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
import sys
from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
import time

def ghz(n_qubits):
    ops = [(gates.H, (0,))]
    for i in range(1, n_qubits):
        ops.append(
            #(gates.CNOT, (i - 1, i))
            (gates.CNOT, (0, i))
        )
    return ops 


def main(n_qubits=100, max_qubits=16000):
    start_time = time.time()

    # Create operation sequence
    ghz_seq = ghz(n_qubits)
    ops = OperationSequence(len(ghz_seq))
    for opcode, args in ghz_seq:
        ops.append(opcode, *args)

    seq_time = time.time()    
    # Create widget, do not teleport input
    wid = Widget(n_qubits, max_qubits)

    # Apply operation on widget
    wid(ops)

    wid.tableau_print()

    widget_io_time = time.time()
    ## Decompose widget
    wid.decompose()
    print("####")

    wid.tableau_print()


    decomposition_time = time.time()

    #json = wid.json()

    json_time = time.time()

    print(f"Python operation creation time: {seq_time - start_time:.4f}")
    print(f"Widget Input Time: {widget_io_time - seq_time:.4f}")
    print(f"Widget Decomposition Time: {decomposition_time - widget_io_time:.4f}")
    print(f"Json Creation Time: {json_time - decomposition_time:.4f}")

    return #json 

if __name__ == '__main__':
    n_qubits = 8 
    max_qubits = 16 
    if len(sys.argv) > 2:
        n_qubits = int(sys.argv[1])
        max_qubits = int(sys.argv[2])
    main(n_qubits, max_qubits)
