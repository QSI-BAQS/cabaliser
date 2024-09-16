from ctypes import cdll, Structure, Union, c_int, c_char, POINTER
import struct
import numpy as np
from operation_sequence import OperationSequence, AdjacencyType, WidgetType, LocalCliffordType, MeasurementTagType

# TODO: Relative import paths and wrap in a package
lib = cdll.LoadLibrary('../cjab.so')
lib.widget_create.restype = POINTER(WidgetType) 


class Widget():
    def __init__(self, n_qubits : int, n_qubits_max : int): 
        '''
            __init__
            Constructor for the widget
            Allocates a large tableau
        '''
        self.__decomposed = False
        self.widget = lib.widget_create(n_qubits, n_qubits_max)

        self.local_cliffords = None
        self.measurements = None
        self.io_map = None 

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
            Parses an array of operations
            :: operations : Operations :: Wrapper around an array of operations  
            Acts in place on the widget
        '''
        lib.parse_instruction_block(
            self.widget,
            operations.ops,
            operations.curr_instructions)
        return

    def __call__(self, *args, **kwargs):
        '''
            Consumes a list of operations
        '''
        self.process_operations(*args, **kwargs)


    def __del__(self):
        ''' 
            __del__
            Explicit destructor for the widget
            Frees the underlying C object
        '''
        lib.widget_destroy(self.widget)

    def get_local_cliffords(self):
        '''
            get_corrections
            Returns a wrapper around an array of the Pauli corrections 
        '''
        if not self.__decomposed:
            raise Exception("Attempted to read out the graph state without decomposing the tableau, please call `Widget.decompose()` before extracting the adjacencies")

        if self.local_cliffords is None:

            local_cliffords = POINTER(LocalCliffordType)() 
            ptr = POINTER(LocalCliffordType)(local_cliffords)
            lib.widget_get_local_cliffords_api(self.widget, ptr)
            self.local_cliffords = LocalCliffords(self.get_n_qubits(), local_cliffords)

        return self.local_cliffords

    def get_measurement_tags(self):
        '''
            get_measurement_tags
            Returns a wrapper around an array of measurements 
        '''
        if not self.__decomposed:
            raise Exception("Attempted to read out the graph state without decomposing the tableau, please call `Widget.decompose()` before extracting the adjacencies")

        if self.local_cliffords is None:

            local_cliffords = POINTER(LocalCliffordType)() 
            ptr = POINTER(LocalCliffordType)(local_cliffords)
            lib.widget_get_local_cliffords_api(self.widget, ptr)
            self.local_cliffords = LocalCliffords(self.get_n_qubits(), local_cliffords)

        return self.local_cliffords

    def get_adjacencies(self, qubit: int):
        '''
            Given a qubit get the adjacencies on the graph
            Reported qubits are graph state indices
        '''
        if not self.__decomposed:
            raise Exception("Attempted to read out the graph state without decomposing the tableau, please call `Widget.decompose()` before extracting the adjacencies")

        if qubit > self.get_n_qubits():
            raise IndexError("Attempted to access qubit out of range") 

        # Honestly this is cleaner than manually setting return types on globally scoped objects
        adjacency_obj = AdjacencyType() 
        ptr = POINTER(AdjacencyType)(adjacency_obj)
        lib.widget_get_adjacencies_api(self.widget, qubit, ptr)
        adj = Adjacency(adjacency_obj)

        return adj 

    def decompose(self):
        if self.__decomposed:
            raise Exception("Attempted to decompose twice")
        lib.widget_decompose(self.widget)
        self.__decomposed = True


class Measurements(QubitArray): 
    pass

class LocalCliffords(QubitArray): 
    pass

  
 
class Adjacency(QubitArray): 
    '''
        Adjacency
        Python wrapper for adjacency object
    '''
    def __init__(self, adj : AdjacencyType):
        self.adj = adj 
        self.src = int(self.adj.src)
        self.n_adjacent = int(self.adj.n_adjacent)
        super().__init__(self.n_adjacent, self.adj.adjacencies)

    def __iter__(self):
        for i in range(self.n_adjacent):
            yield self.adj.adjacencies[i]

    def __getitem__(self, idx : int):
        if idx > self.n_adjacent:
            raise IndexError("Adjacency index is out of range")  

    def __len__(self):
        return self.adj.n_elements

    def __repr__(self):
        return f"Qubit: {self.src} {list(iter(self))}"

    def __str__(self):
        return self.__repr__()

    def __del__(self):
        lib.widget_destroy_adjacencies(
            POINTER(AdjacencyType)(self.adj)
        )
