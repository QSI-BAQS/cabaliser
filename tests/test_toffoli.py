'''
    Test of Toffoli gate decomposition
    Compare to figures in https://arxiv.org/abs/2209.07345
    Output is identical to up to a permutation of qubit indicies
    for the |000> state 
'''
import unittest

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget

# Rz tags
_I_ = 0
_T_ = 1
_Tdag_ = 2

class ToffoliTest(unittest.TestCase):

    # Parameterised toffoli gate
    @staticmethod
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
        (gates.CNOT, (ctrl_a, ctrl_b)),
        (gates.RZ, (ctrl_a, _T_)),
        (gates.RZ, (ctrl_b, _Tdag_)),
        (gates.CNOT, (ctrl_a, ctrl_b)),
        (gates.H, (targ,))
    ]

    def test_toffoli_gate(self, n_qubits=3, max_qubits=64):
        # Create operation sequence
        toffoli = self.toffoli_gate(0, 1, 2)
        ops = OperationSequence(len(toffoli))
        for opcode, args in toffoli:
            ops.append(opcode, *args)

        # Create widget, do not teleport input
        wid = Widget(n_qubits, max_qubits, teleport_input=False)

        # Apply operation on widget
        wid(ops)

        # Decompose widget
        wid.decompose()

        # Test Adjacencies
        for i in (0, 1, 7, 8):
            assert 0 == wid.get_adjacencies(i).n_adjacent
        assert 4 == wid.get_adjacencies(2).n_adjacent
        for i in range(3, 7):
            assert 1 == wid.get_adjacencies(i).n_adjacent

        # Test Measurement tags
        measurements = [_T_, _Tdag_, _Tdag_, _T_, _Tdag_, _T_, _I_, _I_, _I_]
        for m, tag in zip(measurements, wid.get_measurement_tags()):
            assert m == tag

        # Test IO
        measurements = [_T_, _Tdag_, _Tdag_, _T_, _Tdag_, _T_, _I_, _I_, _I_]
        for m, tag in zip(measurements, wid.get_measurement_tags()):
            assert m == tag

        io_map = [7, 8, 6]
        for i, j in zip(io_map, wid.get_io_map()):
            assert i == j


    def test_toffoli_gate_teleported_input(self, n_qubits=3, max_qubits=1024):
        # Create operation sequence
        toffoli = self.toffoli_gate(0, 1, 2)
        ops = OperationSequence(len(toffoli))
        for opcode, args in toffoli:
            ops.append(opcode, *args)

        # Create widget teleport input
        wid = Widget(n_qubits, max_qubits)

        # Apply operation on widget
        wid(ops)

        # Decompose widget
        wid.decompose()

        assert wid.n_qubits == 12


    def test_long_toffoli_sequence(self, n_reps=10, n_qubits=3, max_qubits=None):

        if max_qubits is None:
            max_qubits = n_reps * 10

        # Create operation sequence
        toffoli = self.toffoli_gate(0, 1, 2)
        ops = OperationSequence(len(toffoli))
        for opcode, args in toffoli:
            ops.append(opcode, *args)

        # Create widget teleport input
        wid = Widget(n_qubits, max_qubits)


        for _ in range(n_reps):
            # Apply operation on widget
            wid(ops)

        # Decompose widget
        wid.decompose()

        assert wid.n_qubits == 6 * n_reps + 6 

    def test_range_of_sequences(self):
        '''
            This should push from a single allocated chunk to multiple
        '''
        for i in range(20, 100, 7):
            self.test_long_toffoli_sequence(n_reps=i) 


    def test_very_large_sequences(self):
        '''
            Large numbers of chunks
        '''
        self.test_long_toffoli_sequence(n_reps=2024) 


if __name__ == '__main__':
    unittest.main()
