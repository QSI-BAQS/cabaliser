'''
    Wrapper object for Ctype arrays
    Exposes some simple python interfaces
'''


class QubitArray:
    '''
        QubitArray
        Python wrapper for Ctype arrays
    '''
    def __init__(self, n_qubits : int, arr):
        '''
            Initialiser
            :: n_qubits : int :: Number of elements in the array
            :: arr : POINTER(<T>) :: Pointer object to the array
        '''
        self.n_qubits = n_qubits
        self.arr = arr

    def __iter__(self):
        '''
            Generator style __iter__
            Yields elements of the array in order
        '''
        for i in range(self.n_qubits):
            yield self.arr[i]

    def __getitem__(self, idx : int):
        '''
            Returns an item from the array
            :: idx : int :: Index to query
        '''
        if idx > self.n_qubits:
            raise IndexError("Qubit index is out of range")
        return self.arr[idx]

    def __len__(self):
        '''
            Returns the length of the array
        '''
        return self.n_qubits

    def to_list(self):
        '''
            Converts the underlying array to a Python list
        '''
        return list(iter(self))
