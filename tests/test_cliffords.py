import unittest

from itertools import combinations_with_replacement, permutations, chain
from functools import reduce

from math import isnan

import numpy as np

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence
from cabaliser.widget import Widget
from cabaliser import local_simulator

MAX_CLIFFORD_DEPTH = 12
EPS = 1e-7
N_REPETITIONS = 10

class CliffordsTest(unittest.TestCase):

#    def test_clifford_depth_1(self):
#        self.clifford_sequences(depth=1, apply=False, test_gates=True)
#
#    def test_clifford_depth_1_applied(self):
#        self.clifford_sequences(depth=1, apply=True, test_gates=False)
#
#    def test_clifford_depth_2(self):
#        self.clifford_sequences(depth=2, test_gates=True)
#
#    def test_clifford_depth_3(self):
#        self.clifford_sequences(depth=3, test_states=False, test_gates=True) # Floating point kicks in around here
#
#    def test_clifford_depth_4(self):
#       self.clifford_sequences(depth=4, test_states=False, test_gates=True)
#
#    def test_clifford_depth_5(self): # Past depth 5 exhaustive search's exponential scaling hits hard
#        self.clifford_sequences(depth=5, test_states=False, test_gates=True)

#    def test_clifford_depth_2_applied(self):
#        self.clifford_sequences(depth=2, apply=True, test_gates=False, test_states=True)
#
    def test_clifford_depth_3_applied(self):
        self.clifford_sequences(depth=3, apply=True, test_gates=False, test_states=True) # Floating point kicks in around here
#
#    def test_cnot_clifford_depth_1(self):
#        self.clifford_sequences_cnot_first(depth=1)
#
#    def test_cnot_clifford_depth_2(self):
#        self.clifford_sequences_cnot_first(depth=2)
#
#    def test_cz_clifford_depth_1(self):
#        self.clifford_sequences_cz_first(depth=1)

    def clifford_sequences(self, depth=1, test_states=True, test_gates=True, apply=False):
        '''
           Sequence of random local cliffords
        '''
        n_qubits = 1
        max_qubits = 3

        # Test all two qubit clifford sequences
        for op_seq in combinations_with_replacement(gates.SINGLE_QUBIT_GATES, depth):
            for op_perm in permutations(op_seq):
                operations = [(op, (0,)) for op in op_perm]

                wid = Widget(n_qubits, max_qubits)

                ops = OperationSequence(len(operations))
                for opcode, args in operations:
                    ops.append(opcode, *args)

                operation = reduce(
                    lambda a, b: b @ a,
                    map(lambda x: local_simulator.LOCAL_CLIFFORD_OP_TABLE[x[0]], operations),
                    local_simulator.I
                 )

                wid(ops)

                if apply:
                    wid.apply_local_cliffords()

                print("Pre:")
                wid.tableau_print_phases()
                wid.tableau_print()


                wid.decompose()

                if wid.get_local_cliffords().to_list()[0] != 'I':
                    print("Post:")
                    wid.tableau_print()
                    print(ops)
                    print(wid.get_local_cliffords().to_list())


                if test_gates:  # Only tests the clifford composition sequences
                    wid_operation = wid.get_local_cliffords().to_list()[1]
                    if not cmp_gates(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation], operation):

                        print("OPs:", list(map(gates.SINGLE_QUBIT_GATE_ARR.__getitem__, map(lambda x: x[0], operations))))
                        print("Operation: ", operation)
                        print(wid_operation)
                        print(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation])
                        print(wid.get_local_cliffords().to_list())

                        assert False

                if test_states:
                    ## Test application to random states
                    for _ in range(N_REPETITIONS):
                        input_state = local_simulator.state_prep(
                           *list(np.random.random(2))
                        )

                        widget_state = local_simulator.simulate_widget(
                             wid,
                             input_state=input_state,
                             trace_graph=True
                        )
                        effective_state = (
                             operation @ input_state
                        )

                        dephase(widget_state)
                        dephase(effective_state)

                        if (np.abs(effective_state - widget_state) > EPS).any():
                            print("Operation: ", operation)
                            print(input_state)
                            print(effective_state)
                            print(widget_state)
                            assert False


    def test_random_cliffords(self, seed=0):
        generator = np.random.default_rng(seed)
        n_qubits = 1
        max_qubits = 3
        for _ in range(N_REPETITIONS):
            operations = list(map(lambda x: (x, (0,)), generator.choice(
            tuple(gates.SINGLE_QUBIT_GATES),
            MAX_CLIFFORD_DEPTH,
            replace=True)))

            wid = Widget(n_qubits, max_qubits)

            ops = OperationSequence(len(operations))
            for opcode, args in operations:
                ops.append(opcode, *args)

            operation = reduce(
                lambda a, b: b @ a,
                map(lambda x: local_simulator.LOCAL_CLIFFORD_OP_TABLE[x[0]], operations),
                local_simulator.I
             )

            wid(ops)
            wid.decompose()

            wid_operation = wid.get_local_cliffords().to_list()[1]
            if not cmp_gates(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation], operation):

                print("OPs:", list(map(gates.SINGLE_QUBIT_GATE_ARR.__getitem__, map(lambda x: x[0], operations))))
                print("Operation: ", operation)
                print(wid_operation)
                print(local_simulator.LOCAL_CLIFFORD_TABLE[wid_operation])
                print(wid.get_local_cliffords().to_list())

                assert False

    def clifford_sequences_cnot_first(self, depth=1, test_states=True):
        '''
           Sequence of random local cliffords on one qubit, followed by a CNOT
        '''
        n_qubits = 2
        max_qubits = 4

        for op_seq in combinations_with_replacement(gates.SINGLE_QUBIT_GATES, depth):
            for op_perm in permutations(op_seq):
                operations = [(op, (0,)) for op in op_perm]
                operations.append((gates.CNOT, (0, 1)))

                wid = Widget(n_qubits, max_qubits)

                ops = OperationSequence(len(operations))
                for opcode, args in operations:
                    ops.append(opcode, *args)

                operation = reduce(
                    lambda a, b: b @ a,
                    map(lambda x: local_simulator.LOCAL_CLIFFORD_OP_TABLE[x[0]], operations[:-1]),
                    local_simulator.I
                 )
                operation = local_simulator.CNOT(2, 0, 1) @ local_simulator.kr(operation, local_simulator.I)

                wid(ops)
                wid.decompose()

                ## Test application to random states
                for _ in range(N_REPETITIONS):
                    input_state = local_simulator.kr(local_simulator.zero_state, local_simulator.zero_state)

                    widget_state = local_simulator.simulate_widget(
                         wid,
                         input_state=input_state,
                         trace_graph=True
                    )
                    effective_state = (
                         operation @ input_state
                    )

                    dephase(widget_state)
                    dephase(effective_state)

                    if (np.abs(effective_state - widget_state) > EPS).any():
                        print("####")
                        print("Operation: ", ops, len(ops))
                        print("####")

                        print({i: ops[i] for i in range(len(ops))})

                        print(local_simulator.vec(input_state))
                        print(local_simulator.vec(effective_state))
                        print(local_simulator.vec(widget_state))
                        assert False


    def clifford_sequences_cz_first(self, depth=1, test_states=True):
        '''
           Sequence of random local cliffords on one qubit, followed by a CZ
        '''
        n_qubits = 2
        max_qubits = 4

        for op_seq in combinations_with_replacement(gates.SINGLE_QUBIT_GATES, depth):
            for op_perm in permutations(op_seq):
                operations = [(op, (0,)) for op in op_perm]
                operations.append((gates.CZ, (0, 1)))

                wid = Widget(n_qubits, max_qubits)

                ops = OperationSequence(len(operations))
                for opcode, args in operations:
                    ops.append(opcode, *args)

                operation = reduce(
                    lambda a, b: b @ a,
                    map(lambda x: local_simulator.LOCAL_CLIFFORD_OP_TABLE[x[0]], operations[:-1]),
                    local_simulator.I
                 )
                operation = local_simulator.CZ(2, 0, 1) @ local_simulator.kr(operation, local_simulator.I)

                wid(ops)
                wid.decompose()

                ## Test application to random states
                for _ in range(N_REPETITIONS):
                    input_state = local_simulator.kr(
                            local_simulator.state_prep(
                                *list(np.random.random(2))
                            ),
                            local_simulator.plus_state
                        )

                    widget_state = local_simulator.simulate_widget(
                         wid,
                         input_state=input_state,
                         trace_graph=True
                    )
                    effective_state = (
                         operation @ input_state
                    )

                    dephase(widget_state)
                    dephase(effective_state)

                    if (np.abs(effective_state - widget_state) > EPS).any():
                        print("Operation: ", ops, len(operation))
                        print(input_state)
                        print(effective_state)
                        print(widget_state)
                        assert False

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

def dephase(vector: np.array) -> bool:
    '''
    Divide whole vector by first non-zero element
    Returns False if the vector is empty, and True
    if the operation completed successfully

    Acts in place on the vector object
    '''
    try:
        val = next(
                    filter(
                        lambda x: np.abs(x) > EPS,
                        chain(iter(vector))
                    )
              )
        if isnan(val[0].real):
            return False
        vector /= val
        return True
    except StopIteration:  # Whole vector empty
        return False


if __name__ == '__main__':
    unittest.main()
