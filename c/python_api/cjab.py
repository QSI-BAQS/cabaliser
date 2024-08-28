from ctypes import cdll
from typing import Final
import struct
import numpy as np


# TODO: Relative import paths and wrap in a package
lib = cdll.LoadLibrary('../cjab.so')

# Gate constants
LOCAL_CLIFFORD_MASK : Final[np.uint8] = np.uint8(1 << 5) 
NON_LOCAL_CLIFFORD_MASK : Final[np.uint8] = np.uint8(1 << 6) 
RZ_MASK : Final[np.uint8] = np.uint8(1 << 7) 

# Set of legal gates
_I_ : Final[np.uint8] = (0x00 | LOCAL_CLIFFORD_MASK)
_X_ : Final[np.uint8] = (0x01 | LOCAL_CLIFFORD_MASK)
_Y_ : Final[np.uint8] = (0x02 | LOCAL_CLIFFORD_MASK)
_Z_ : Final[np.uint8] = (0x03 | LOCAL_CLIFFORD_MASK)
_H_ : Final[np.uint8] = (0x04 | LOCAL_CLIFFORD_MASK)
_S_ : Final[np.uint8] = (0x05 | LOCAL_CLIFFORD_MASK)
_Sd_ : Final[np.uint8] = (0x06 | LOCAL_CLIFFORD_MASK)

# These are expected output values and may not be used as gates  
_HX_ : Final[np.uint8] = (0x07 | LOCAL_CLIFFORD_MASK)
_SX_ : Final[np.uint8] = (0x08 | LOCAL_CLIFFORD_MASK)
_SdX_ : Final[np.uint8] = (0x09 | LOCAL_CLIFFORD_MASK)
_HY_ : Final[np.uint8] = (0x0a | LOCAL_CLIFFORD_MASK)
_HZ_ : Final[np.uint8] = (0x0b | LOCAL_CLIFFORD_MASK)
_SH_ : Final[np.uint8] = (0x0c | LOCAL_CLIFFORD_MASK)
_SdH_ : Final[np.uint8] = (0x0d | LOCAL_CLIFFORD_MASK)
_HS_ : Final[np.uint8] = (0x0e | LOCAL_CLIFFORD_MASK)
_HSd_ : Final[np.uint8] = (0x0f | LOCAL_CLIFFORD_MASK)
_HSX_ : Final[np.uint8] = (0x10 | LOCAL_CLIFFORD_MASK)
_HRX_ : Final[np.uint8] = (0x11 | LOCAL_CLIFFORD_MASK)
_SHY_ : Final[np.uint8] = (0x12 | LOCAL_CLIFFORD_MASK)
_SdHY_ : Final[np.uint8] = (0x13 | LOCAL_CLIFFORD_MASK)
_HSH_ : Final[np.uint8] = (0x14 | LOCAL_CLIFFORD_MASK)
_HRH_ : Final[np.uint8] = (0x15 | LOCAL_CLIFFORD_MASK)
_SdHS_ : Final[np.uint8] = (0x16 | LOCAL_CLIFFORD_MASK)
_SHSd_ : Final[np.uint8] = (0x17 | LOCAL_CLIFFORD_MASK)
      
_CNOT_ : Final[np.uint8] = (0x00 | NON_LOCAL_CLIFFORD_MASK) 
_CZ_ : Final[np.uint8] = (0x01 | NON_LOCAL_CLIFFORD_MASK) 
_RZ_ : Final[np.uint8] = (RZ_MASK)

class Widget():
    def __init__(self, n_qubits : int, n_qubits_max : int): 
        '''
            __init__
            Constructor for the widget
            Allocates a large tableau
        '''
        self.n_qubits = n_qubits
        self.n_qubits_max = n_qubits_max

        self.widget = lib.widget_create(self.n_qubits, self.n_qubits_max) 


    def __del__(self):
        ''' 
            __del__
            Explicit destructor for the widget
            Frees the underlying C object
        '''
        lib.widget_destroy(self.widget)

class Operation(): 
    def __init__(self, op : np.uint8, arg_a : np.uint64, arg_b : np.uint64): 
        self.opcode = op
        self.arg_a = arg_a
        self.arg_b = arg_b

    def pack(self):
        struct.pack('@cNN', self.opcode, self.arg_a, self.arg_b)

