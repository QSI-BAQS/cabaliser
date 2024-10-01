'''
    C struct wrappers as type declarations  
'''
from cabaliser.gates import SINGLE_QUBIT_GATES, TWO_QUBIT_GATES, RZ

from ctypes import Structure, POINTER, c_int, c_byte, c_size_t, POINTER

class CliffordQueueType(Structure):
    '''
      ctypes wrapper for clifford queue structs 
    '''
    _fields_ = [
        ('n_qubits', c_size_t),
        ('cliffords', POINTER(c_byte)),
        ('measurement_tags', POINTER(c_int))
    ]

class WidgetType(Structure):
    '''
       ctypes wrapper for widget structs 
    '''
    _fields_ = [
        ('n_qubits', c_size_t),
        ('n_initial_qubits', c_size_t),
        ('max_qubits', c_size_t),
        ('__tableau', POINTER(c_size_t)),
        ('queue', POINTER(CliffordQueueType)),
        ('map', POINTER(c_size_t))
    ]

LocalCliffordType = c_byte # 1 byte
MeasurementTagType = c_int # 4 bytes
AdjacencyEdgeType = c_int # 4 bytes
IOMapType = c_size_t # 8 bytes

class AdjacencyType(Structure):
    '''
        ctypes wrapper for array of adjacent edges
    '''
    _fields_ = [
        ('src', c_int),
        ('n_adjacent', c_int),
        ('adjacencies', POINTER(AdjacencyEdgeType))]
