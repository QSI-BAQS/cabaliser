'''
    Corresponding structs and Python Classes
'''
from ctypes import Structure, Union, c_int, c_byte


class SingleQubitOperationType(Structure):
    '''
        ctypes wrapper for single qubit operation
    '''
    _fields_ = [
        ('opcode', c_byte),
        ('arg', c_int),
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


class TwoQubitOperationType(Structure):
    '''
        ctypes wrapper for two qubit operation
    '''
    _fields_ = [
        ('opcode', c_byte),
        ('ctrl', c_int),
        ('targ', c_int),
        ]


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
    arr[idx].single.opcode = opcode
    arr[idx].single.ctrl = ctrl
    arr[idx].single.targ = targ


class RzQubitOperationType(Structure):
    '''
        ctypes wrapper for rz operations
    '''
    _fields_ = [
        ('opcode', c_byte),
        ('arg', c_int),
        ('tag', c_int),
        ]


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


class OperationType(Union):
    '''
        ctypes wrapper for general operation
    '''
    _fields_ = [
        ('single', SingleQubitOperationType),
        ('two_qubits', TwoQubitOperationType),
        ('rz', RzQubitOperationType),
        ]
