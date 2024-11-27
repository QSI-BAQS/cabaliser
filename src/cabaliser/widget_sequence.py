from cabaliser.widget import Widget
from cabaliser.operation_sequence import OperationSequence


class WidgetSequence():
    '''
        Creates a sequence of widget objects
    '''
    def __init__(self, qubit_width, max_qubits=10000):
        '''
        '''
        self.seq = []
        self._current_wid = None 
        self._current_seq = None
        self._json = []
        self.rz_threshold  = max_qubits - 2 * qubit_width
        self.qubit_width = qubit_width
        self.max_qubits = max_qubits
        assert qubit_width < max_qubits 

    def __call__(self, ops: OperationSequence, progress=True): 
        '''
        '''
        ops_sequence = ops.split(self.rz_threshold)
        for i, seq in enumerate(ops_sequence):
            if progress:
                print(f"\r{i + 1} of {len(ops_sequence)}", flush=True, end='')
            wid = Widget(self.qubit_width, self.max_qubits) 
            wid(seq)
            wid.decompose()
            self._json.append(wid.json())
            del wid

    def json(self):
        return self._json
    
