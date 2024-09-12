from ctypes import cdll, Structure, Union, c_int, c_char, POINTER
import struct
import numpy as np
from operation_sequence import OperationSequence, Adjacency

# TODO: Relative import paths and wrap in a package
lib = cdll.LoadLibrary('../cjab.so')


class Widget():
    def __init__(self, n_qubits : int, n_qubits_max : int): 
        '''
            __init__
            Constructor for the widget
            Allocates a large tableau
        '''
        self.__decomposed = False
        self.widget = lib.widget_create(n_qubits, n_qubits_max) 


    def get_n_qubits(self) -> int:
        '''
            get_n_qubits
            Getter method for the widget
            Returns the current number of allocated qubits on the widget 
        '''
        return lib.widget_get_n_qubits(self.widget)

    def get_max_qubits(self) -> int:
        '''
            get_max_qubits
            Getter method for the widget
            Returns the maximum number of allocatable qubits on the widget 
        '''
        return lib.widget_get_max_qubits(self.widget)

    def process_operations(self, operations):
        '''

        '''
        lib.parse_instruction_block(
            self.widget,
            operations.ops,
            operations.curr_instructions)
        return

    def __call__(self, *args, **kwargs):
        self.process_operations(*args, **kwargs)


    def __del__(self):
        ''' 
            __del__
            Explicit destructor for the widget
            Frees the underlying C object
        '''
        lib.widget_destroy(self.widget)

    def get_adjacencies(self, qubit: int):
        try:
            assert(self.__decomposed)
        except:
            raise Exception("Attempted to read out the graph state without decomposing the tableau, please call `Widget.decompose()` before extracting the adjacencies")
        adjacency_obj = Adjacency(lib.widget_get_adjacencies(self.widget,qubit))
        return adjacency_obj

    def decompose(self):
        if (not self.__decomposed):
            raise Exception("Attempted to decompose twice")
        lib.widget_decompose(self.widget)
        self.__decomposed = True
