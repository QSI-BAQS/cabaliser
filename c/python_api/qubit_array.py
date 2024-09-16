class QubitArray: 
    '''
        QubitArray 
        Python wrapper for Ctype arrays  
    '''
    def __init__(self, n_qubits : int, arr):
        self.n_qubits = n_qubits
        self.arr = arr

    def __iter__(self):
        for i in range(self.n_qubits):
            yield self.arr[i]

    def __getitem__(self, idx : int) -> int:
        if idx > self.n_qubits:
            raise IndexError("Qubit index is out of range")  

    def __len__(self):
        return self.n_qubits

