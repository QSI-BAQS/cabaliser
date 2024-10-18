'''
    Wrapper objects around QubitArray
    Typing is only for verbosity and any potential
    extensibility that might be required
'''
from cabaliser.qubit_array import QubitArray
from cabaliser.structs import ScheduleDependencyType
from cabaliser.gates import SINGLE_QUBIT_GATE_TABLE
from cabaliser.gate_constructors import tag_to_angle

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
    def __init__(self, n_qubits, qubit_index, arr):
        self.qubit_index = qubit_index
        super().__init__(n_qubits, arr)

    @staticmethod
    def from_struct(struct: ScheduleDependencyType):
        return ScheduleDependency(
            struct.len,
            struct.dependent,
            struct.arr
        )

    def __repr__(self):
        return f"({self.qubit_index}: {list(iter(self))})"

    def __str__(self):
        return self.__repr__()
