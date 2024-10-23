import unittest
import numpy as np

import local_simulator

from cabaliser import gates
from cabaliser.operation_sequence import OperationSequence 
from cabaliser.widget import Widget

EPS = 1e-8
N_REPETITIONS = 100
class OperationTest(unittest.TestCase):

    @staticmethod
    def state_prep(alpha, beta):
        state = alpha * local_simulator.zero_state + beta * local_simulator.one_state
        state = local_simulator.norm(state)
        return state 

    @staticmethod
    def simulate_widget(widget, *input_states):
        cz_operations, local_cliffords, measurements = OperationTest.widget_to_operations(widget) 
        n_inputs = len(input_states) 
        if n_inputs > widget.n_initial_qubits: 
            raise Exception("More input states than inputs")

        graph_state = local_simulator.kr(
            *[local_simulator.plus_state] * widget.n_qubits,
            *input_states)
        higher_hilbert_space = local_simulator.kr(*[local_simulator.I] * len(input_states)) 

        cz_operations = local_simulator.kr(cz_operations, higher_hilbert_space) 
        local_cliffords = local_simulator.kr(local_cliffords, higher_hilbert_space)
        measurements = local_simulator.kr(measurements, higher_hilbert_space)

        input_operations = local_simulator.inject_inputs(widget.n_qubits, n_inputs)

        return local_simulator.norm(
                measurements
                @ local_cliffords
                @ input_operations
                @ cz_operations
                @ graph_state
            )
            

    @staticmethod
    def widget_to_operations(widget):
        psi = 0.9 ** 2 * local_simulator.zero_state + 0.1 ** 2 * local_simulator.one_state 

        initial_state = local_simulator.kr(*[local_simulator.plus_state] * widget.n_qubits) 
        cz_operations = local_simulator.widget_to_cz(widget) 
        local_cliffords = local_simulator.local_cliffords(widget)  

        measurements = local_simulator.measurement_schedule(widget) 

        return cz_operations, local_cliffords, measurements 


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
            input_state = self.state_prep(*list(np.random.random(2)))
            widget_state = self.simulate_widget(wid, input_state)
       
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
            input_state = self.state_prep(*list(np.random.random(2)))
            widget_state = self.simulate_widget(wid, input_state)
       
            # Output should be the CNOT of the input state on qubits 2 and 3 
            zero_state = local_simulator.zero_state 
            effective_state = (
                local_simulator.CNOT(n_qubits, 0, 1)
                @ local_simulator.kr(input_state, zero_state) 
            )
            embedded_state = local_simulator.kr(zero_state, zero_state, effective_state, zero_state) 
           
            assert ((embedded_state - widget_state) < EPS).all()

if __name__ == '__main__':
    unittest.main()
