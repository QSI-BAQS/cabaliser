'''
    Wrapper objects around QubitArray
    Typing is only for verbosity and any potential
    extensibility that might be required
'''

from ctypes import POINTER

from cabaliser.qubit_array import QubitArray
from cabaliser.structs import ScheduleDependencyType, PauliCorrectionType
from cabaliser.gates import SINGLE_QUBIT_GATE_TABLE
from cabaliser.gate_constructors import tag_to_angle
from cabaliser.utils import deref
from cabaliser.lib_cabaliser import lib


class MeasurementTags(QubitArray):
    '''
        Ordered array of RZ measurement tags
    '''
    def to_list(self, to_float=False):
        if to_float:
            return list(map(tag_to_angle, self))
        return super().to_list()


class LocalCliffords(QubitArray):
    '''
        Ordered array of local clifford operations
    '''
    def to_list(self, to_string=True):
        if to_string:
            return list(map(SINGLE_QUBIT_GATE_TABLE.__getitem__, self))
        return super().to_list()


class IOMap(QubitArray):
    '''
        Ordered array of map of input qubits to output qubits
    '''


class ScheduleDependency(QubitArray):
    '''
        ScheduleDependency
        Wrapper for the rustlib pauli tracker schedule
    '''
    def __init__(self, n_qubits, qubit_index, arr, node_ptr):
        self.qubit_index = qubit_index
        self.__ptr = node_ptr
        super().__init__(n_qubits, arr)

    @staticmethod
    def from_struct(struct: ScheduleDependencyType):
        return ScheduleDependency(
            struct.len,
            struct.dependent,
            struct.arr
        )

    def to_dict(self):
        '''
            Returns a dictionary object of the dependencies 
        '''
        return {self.qubit_index: self.to_list()}

    def __repr__(self):
        return f"({self.qubit_index}: {list(iter(self))})"

    def __str__(self):
        return self.__repr__()
    def __del__(self):
        pass
        # TODO: This is probably leaking about 16 bytes of memory on python cleanup 
        # lib.lib_pauli_tracker_const_vec_destroy(self.__ptr) 

class PauliCorrection(QubitArray):

    pauli_to_str = {
        0: 'I',
        1: 'Z',
        2: 'X', 
        3: 'Y'    
    }

    def __init__(self, index, correction: POINTER(PauliCorrectionType)):
        self.__ptr = correction
        self.cap = deref(correction).cap
        self.qubit_index = index

        self.__list = None

        super().__init__(deref(correction).len, deref(correction).arr) 

    def __del__(self):
        '''
            TODO:
            The underlying array object is a shared pointer
            Capacity and len should be set to zero to ensure that the array is not freed by rust  
        '''
        pass

    def to_list(self, cache: bool = True):
        '''
            Converts the array to a Python list
            :: cache : bool :: Whether to cache the list 
        '''
        if cache and self.__list is None:        
            self.__list = list(map(
                self.pauli_to_str.get,
                super().to_list(cache=cache)
            ))
        elif not cache:
            return list(map(
                self.pauli_to_str.get,
                super().to_list()
            ))
        return self.__list

    def to_dict(self, cache=True):
        '''
            Converts the array to a Python dict 
            :: cache : bool :: Whether to cache the underlying list 
        '''

        return {self.qubit_index: self.to_list(cache=cache)}

    def to_tuple(self, cache=True):
        '''
            Converts the array to a Python tuple 
            :: cache : bool :: Whether to cache the underlying list 
        '''
        return (self.qubit_index, self.to_list(cache=cache))

    def __repr__(self):
        return self.to_dict().__repr__()
    
    def __str__(self):
        return self.__repr__()
