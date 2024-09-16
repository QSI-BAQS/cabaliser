from ctypes import cdll, Structure, Union, c_int, c_byte, c_size_t, POINTER
import struct

from gates import SINGLE_QUBIT_GATES, TWO_QUBIT_GATES, _RZ_ 

class CliffordQueueType(Structure):
    _fields_ = [
        ('n_qubits', c_size_t), 
        ('cliffords', POINTER(c_byte)), 
        ('measurement_tags', POINTER(c_int)) 
    ]


class WidgetType(Structure):
    _fields_ = [
        ('n_qubits', c_size_t), 
        ('n_initial_qubits', c_size_t), 
        ('max_qubits', c_size_t), 
        ('__tableau', POINTER(c_size_t)), 
        ('queue', POINTER(CliffordQueueType)), 
        ('map', POINTER(c_size_t)) 
    ]


class SingleQubitOperationType(Structure): 
    '''
        ctypes wrapper for single qubit operation
    '''
    _fields_ = [
        ('opcode', c_byte), 
        ('arg', c_int), 
        ]

class TwoQubitOperationType(Structure): 
    '''
        ctypes wrapper for two qubit operation
    '''
    _fields_ = [
        ('opcode', c_byte), 
        ('ctrl', c_int), 
        ('targ', c_int), 
        ]

class RzQubitOperationType(Structure): 
    '''
        ctypes wrapper for rz operations
    '''
    _fields_ = [
        ('opcode', c_byte), 
        ('arg', c_int), 
        ('tag', c_int), 
        ]

class OperationType(Union): 
    '''
        ctypes wrapper for general operation 
    '''
    _fields_ = [
        ('single', SingleQubitOperationType), 
        ('two_qubits', TwoQubitOperationType), 
        ('rz', RzQubitOperationType), 
        ]


def SingleQubitOperation(arr, idx, opcode, arg):
    '''
       Constructor for single qubit operations 
        :: arr : array :: Array to write to 
        :: idx : int :: Index to write to
        :: opcode : uint8_t :: Opcode for single qubit operation 
        :: arg : uint32_t :: Target qubit 
        Writes the operation to the index of the array
    '''
    arr[idx].single.opcode = opcode
    arr[idx].single.arg = arg

def TwoQubitOperation(arr, idx, opcode, ctrl, targ):
    '''
       Constructor for two qubit operations 
        :: arr : array :: Array to write to 
        :: idx : int :: Index to write to
        :: opcode : uint8_t :: Opcode for single qubit operation 
        :: ctrl : uint32_t :: First qubit argument 
        :: targ : uint32_t :: Second qubit argument 
        Writes the operation to the index of the array
    '''

    arr[idx].single.opcode = opcode
    arr[idx].single.ctrl = ctrl
    arr[idx].single.targ = targ

def RzOperation(arr, i, opcode, arg, tag):
    '''
       Constructor for two qubit operations 
        :: arr : array :: Array to write to 
        :: idx : int :: Index to write to
        :: opcode : uint8_t :: RZ opcode 
        :: arg : uint32_t :: Qubit argument 
        :: tag : uint32_t :: Tag for the rz angle  
        Writes the operation to the index of the array
    '''
    arr[i].rz.opcode = opcode
    arr[i].rz.arg = arg
    arr[i].rz.tag = tag


LocalCliffordType = c_byte # 1 byte
MeasurementTagType = c_int # 4 bytes
AdjacencyEdgeType = c_int # 4 bytes
IOMapType = c_int # 4 bytes
class AdjacencyType(Structure):
    '''
        ctypes wrapper for array of adjacent edges
    '''
    _fields_ = [
        ('src', c_int),
        ('n_adjacent', c_int),
        ('adjacencies', POINTER(AdjacencyEdgeType))]


# Map from gates symbols to constructors
class OperationSequence():
    '''
        Operation sequence object
        This is a wrapper for an array of ctypes operations
        :: n_instructions : int :: Maximum number of instructions for this sequence 
        This object has a pre-allocated maximum number of supported operations 
    '''
    CONSTRUCTOR_MAP = (
          {i: SingleQubitOperation for i in SINGLE_QUBIT_GATES} 
        | {i: TwoQubitOperation for i in TWO_QUBIT_GATES} 
        | {_RZ_ : RzOperation})

    def __init__(self, n_instructions : int):
        '''
        Constructor for the operation sequence object 
        :: n_instructions : int :: Maximum number of instructions for this sequence 
        '''
        self.n_instructions = n_instructions
        self.ops = (OperationType * n_instructions)()
        self.curr_instructions = 0

    def __getitem__(self, idx : int):
        '''
            __getitem__
            Gets an operation from the array
            :: idx : int :: index to query 
            Warning: negative integers will return the value from negative addresses, they will NOT index from the end of the array 
        '''
        return self.ops[idx] 

    def __setitem__(self, idx, value):
        '''
            __setitem__
            Sets an operation from the array
            :: idx : int :: index to query 
            :: value : OperationType :: The operation to write to that element
            Warning: negative integers will write to addresses below the address of the array, they will NOT index from the end of the array 
        '''

        self.ops[idx] = value 

    def __iter__(self):
        '''
            __iter__
            Iterator for the OperationSequence object
            Iterates over the set of currently set operations
        '''
        for i in range(self.curr_instructions):
            yield self.ops[i]
        return

    def append(self, opcode, *args): 
        if self.curr_instructions == self.n_instructions:
            raise Exception("Exceeded Length of Array")

        self.CONSTRUCTOR_MAP[opcode](self.ops, self.curr_instructions, opcode, *args)  
        self.curr_instructions += 1
        
    def _append(self, *operations):
        if self.curr_instructions + len(operations) > self.n_instructions:  
            raise Exception("Exceeded Length of Array")

        for operation in operations:
            self[self.curr_instructions] = operation
            self.curr_instructions += 1

    def __add__(self, other):   
        '''
            __add__
            Helper function to make these arrays more list-like
            :: other : OperationSequence :: Another operation sequence 
            Returns a new operation sequence containing lhs operations then rhs operations
        '''
        seq = OperationSequence(self.n_instructions + other.n_instructions) 
        # This could probably be done with a wrap around memcpy
        for op in self:
            seq._append(op)
        for op in other:
            seq._append(op)
