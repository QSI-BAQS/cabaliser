import unittest
import numpy as np

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget
from cabaliser import local_simulator 

EPS = 1e-8
N_REPETITIONS = 100
class OperationTest(unittest.TestCase):

    
    def test_hadamard(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.H, (0,))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()
        
        json = wid.json()
        # One input, one output
        assert json['n_qubits'] == 2

        # Hadamard on the output
        assert json['local_cliffords'][1] == 'H'

        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_widget(wid, input_state)
       
            # Output should be the Hadamard of the input state on qubit 2

            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.H @ input_state
            )
            embedded_state = local_simulator.kr(zero_state, effective_state, zero_state) 
        
            assert (embedded_state - widget_state < EPS).all()

    def test_cnot(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.CNOT, (0, 1))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_widget(wid, input_state)
       
            # Output should be the CNOT of the input state on qubits 2 and 3 
            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.CNOT(n_qubits, 0, 1)
                @ local_simulator.kr(input_state, zero_state) 
            )
            embedded_state = local_simulator.kr(zero_state, zero_state, effective_state, zero_state) 
           
            assert ((embedded_state - widget_state) < EPS).all()

    def test_cz(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.CZ, (0, 1))] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_widget(wid, input_state)
       
            # Output should be the CNOT of the input state on qubits 2 and 3 
            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.CZ(n_qubits, 0, 1)
                @ local_simulator.kr(input_state, zero_state) 
            )
            embedded_state = local_simulator.kr(zero_state, zero_state, effective_state, zero_state) 
           
            assert ((embedded_state - widget_state) < EPS).all()

    def test_phase(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.S, [0])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_widget(wid, input_state)
       
            # Output should be the CNOT of the input state on qubits 2 and 3 
            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.kr(local_simulator.S, local_simulator.I)
                @ local_simulator.kr(input_state, zero_state) 
            )
            embedded_state = local_simulator.kr(zero_state, zero_state, effective_state, zero_state) 
            assert ((embedded_state - widget_state) < EPS).all()
   

    def test_phase_dag(self):  
        '''
            Teleport one qubit then perform a Hadamard
        '''

        n_qubits = 2
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.S, [0])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_widget(wid, input_state)
       
            # Output should be the CNOT of the input state on qubits 2 and 3 
            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.kr(local_simulator.Sdag, local_simulator.I)
                @ local_simulator.kr(input_state, zero_state) 
            )
            embedded_state = local_simulator.kr(zero_state, zero_state, effective_state, zero_state) 
            assert ((embedded_state - widget_state) < EPS).all()


    def test_rz_no_rotation(self):  
        '''
            RZ gate triggering a teleport
        '''

        n_qubits = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
        I = 0
    
        operations = [(gates.RZ, [0, I])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_widget(wid, input_state)
       
            # Output should be a T gate acting on qubit 2  
            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.kr(local_simulator.zero_state, input_state) 
            )
            embedded_state = local_simulator.kr(zero_state, effective_state, zero_state) 

            assert ((embedded_state - widget_state) < EPS).all()


    def test_rz(self):  
        '''
            RZ gate triggering a teleport
        '''

        n_qubits = 1
        T = 1
        max_qubits = 10
        wid = Widget(n_qubits, max_qubits)
    
        operations = [(gates.RZ, [0, T])] 
        ops = OperationSequence(len(operations))
        for opcode, args in operations: 
            ops.append(opcode, *args)

        wid(ops) 
        wid.decompose()

        # Test state
        for _ in range(N_REPETITIONS):
            input_state = local_simulator.state_prep(*list(np.random.random(2)))
            widget_state = local_simulator.simulate_widget(wid, input_state)
       
            # Output should be a T gate acting on qubit 2  
            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.kr(local_simulator.I, local_simulator.T)
                @ local_simulator.kr(local_simulator.zero_state, input_state) 
            )
            embedded_state = local_simulator.kr(zero_state, effective_state, zero_state) 

            assert ((embedded_state - widget_state) < EPS).all()


if __name__ == '__main__':
    unittest.main()
