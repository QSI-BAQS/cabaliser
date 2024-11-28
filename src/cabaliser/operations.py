'''
    Corresponding structs and Python Classes
'''
from ctypes import Structure, Union, c_uint32_t, c_uint8_t
from cabaliser.gates import SINGLE_QUBIT_GATES, TWO_QUBIT_GATES

OpcodeType = c_uint8_t

class SingleQubitOperationType(Structure):
    '''
        ctypes wrapper for single qubit operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('arg', c_uint32_t),
        ]

    def __repr__(self):
        return f"Op: {self.arg}"

    def __str__(self):
        return self.__repr__()

    def max_qubits_index(self):
        return int(self.arg)


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


class TwoQubitOperationType(Structure):
    '''
        ctypes wrapper for two qubit operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('ctrl', c_uint32_t),
        ('targ', c_uint32_t),
        ]

    def __repr__(self):
        return f"Op: {self.ctrl} {self.targ}"

    def __str__(self):
        return self.__repr__()

    def max_qubits_index(self):
        return max(int(self.arg), int(self.targ))


def TwoQubitOperation(arr, idx: int, opcode: c_byte, ctrl: int, targ: int):
    '''
       Constructor for two qubit operations
        :: arr : array :: Array to write to
        :: idx : int :: Index to write to
        :: opcode : uint8_t :: Opcode for single qubit operation
        :: ctrl : uint32_t :: First qubit argument
        :: targ : uint32_t :: Second qubit argument
        Writes the operation to the index of the array
    '''
    arr[idx].two_qubits.opcode = opcode
    arr[idx].two_qubits.ctrl = ctrl
    arr[idx].two_qubits.targ = targ


class RzQubitOperationType(Structure):
    '''
        ctypes wrapper for rz operations
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('arg', c_uint32_t),
        ('tag', c_uint32_t),
        ]

    def __repr__(self):
        return f"Rz({self.arg}) : {self.tag}"

    def __str__(self):
        return self.__repr__()

    def max_qubits_index(self):
        return int(self.arg)


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



# This is identical to the two qubit operation 
# But we provide a different class for clarity
class ConditionalOperationType(Structure):
    '''
        ctypes wrapper for two qubit operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('ctrl', c_uint32_t),
        ('targ', c_uint32_t),
        ]

    def __repr__(self):
        return f"Cond Op: {self.ctrl} {self.targ}"

    def __str__(self):
        return self.__repr__()


def ConditionalOperation(arr, i, opcode, ctrl, targ):
    '''
    Constructor for conditional operations
    :: arr : array :: Array to write to
    :: idx : int :: Index
    :: opcode : uint8_t :: Opcode
    :: ctrl : uin32_t :: Control Qubit 
    :: targ : uin32_t :: Target Qubit
    '''
    arr[idx].two_qubits.opcode = opcode
    arr[idx].two_qubits.ctrl = ctrl
    arr[idx].two_qubits.targ = targ


class OperationType(Union):
    '''
        ctypes wrapper for general operation
    '''
    _fields_ = [
        ('opcode', OpcodeType),
        ('single', SingleQubitOperationType),
        ('two_qubits', TwoQubitOperationType),
        ('rz', RzQubitOperationType),
        ('cond_op', ConditionalOperationType), 
        ]

    def __repr__(self):
        opcode = self.single.opcode
        if opcode in SINGLE_QUBIT_GATES:
            return self.single.__repr__()
        if opcode in TWO_QUBIT_GATES:
            return self.two_qubits.__repr__()
        return self.rz.__repr__()

    def is_rz(self):
        return self.single.opcode < 0

