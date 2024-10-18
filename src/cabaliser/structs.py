'''
    C struct wrappers as type declarations
'''
from ctypes import Structure, POINTER, c_int32, c_byte, c_size_t, c_uint32

LocalCliffordType = c_byte  # 1 byte
MeasurementTagType = c_int32  # 4 bytes
AdjacencyEdgeType = c_int32  # 4 bytes
IOMapType = c_size_t  # 8 bytes


class CliffordQueueType(Structure):
    '''
      ctypes wrapper for clifford queue structs
    '''
    _fields_ = [
        ('n_qubits', c_size_t),
        ('cliffords', POINTER(LocalCliffordType)),
        ('measurement_tags', POINTER(MeasurementTagType))
    ]


class MappedPauliTrackerType(Structure):
    '''
        Dummy pointer wrapper
    '''
    _fields_ = [
        ('__', c_int32)
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
        ('map', POINTER(IOMapType)),
        ('pauli_tracker', POINTER(MappedPauliTrackerType))
    ]


class AdjacencyType(Structure):
    '''
        ctypes wrapper for array of adjacent edges
    '''
    _fields_ = [
        ('src', c_int32),
        ('n_adjacent', c_int32),
        ('adjacencies', POINTER(AdjacencyEdgeType))]


class PartialOrderGraphType(Structure):
    '''
        Dummy pointer wrapper
    '''
    _fields_ = [
        ('__', c_int32)
    ]


class ScheduleDependencyType(Structure):
    '''
        Array wrapper type for dependency schedules
    '''
    _fields_ = [
        ('len', c_uint32),
        ('dependent', c_uint32),
        ('arr', POINTER(c_size_t))
    ]
