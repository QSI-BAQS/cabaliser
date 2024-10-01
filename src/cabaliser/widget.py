from ctypes import Structure, Union, c_int, c_char, c_buffer, POINTER
import struct
import numpy as np
from cabaliser.operation_sequence import OperationSequence, AdjacencyType,  WidgetType, LocalCliffordType, MeasurementTagType, IOMapType

from cabaliser.io_array_wrappers import MeasurementTags, LocalCliffords, IOMap
from cabaliser.qubit_array import QubitArray


# TODO: Relative import paths and wrap in a package
from cabaliser.lib_cabaliser import lib
lib.widget_create.restype = POINTER(WidgetType)

class Widget():
    def __init__(self, n_qubits : int, n_qubits_max : int, teleport_input=True):
        '''
            __init__
            Constructor for the widget
            Allocates a large tableau
        '''
        self.__decomposed = False
        self.widget = lib.widget_create(n_qubits, n_qubits_max)
        self.teleport_input = teleport_input

        if self.teleport_input:
            lib.teleport_input(self.widget)

        self.local_cliffords = None
        self.measurement_tags = None
        self.io_map = None

    def get_n_qubits(self) -> int:
        '''
            get_n_qubits
            Getter method for the widget
            Returns the current number of allocated qubits on the widget
        '''
        return lib.widget_get_n_qubits(self.widget)

    @property
    def n_qubits(self):
        return self.get_n_qubits()

    def get_max_qubits(self) -> int:
        '''
            get_max_qubits
            Getter method for the widget
            Returns the maximum number of allocatable qubits on the widget
        '''
        return lib.widget_get_max_qubits(self.widget)

    @property
    def max_qubits(self):
        return self.get_max_qubits()


    def get_initial_qubits(self) -> int:
        '''
            get_initial_qubits
            Getter method for the widget
            Returns the initial number of allocatable qubits on the widget
        '''
        return lib.widget_get_n_initial_qubits(self.widget)

    @property
    def n_initial_qubits(self):
        return self.get_n_initial_qubits()

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

        if self.measurement_tags is None:
            measurement_tags = POINTER(MeasurementTagType)()
            ptr = POINTER(MeasurementTagType)(measurement_tags)
            lib.widget_get_measurement_tags_api(self.widget, ptr)
            self.measurement_tags = MeasurementTags(self.get_n_qubits(), measurement_tags)

        return self.measurement_tags

    def get_io_map(self):
        '''
            get_io_map
            Returns a wrapper around an array of measurements
        '''
        if not self.__decomposed:
            raise Exception("Attempted to read out the graph state without decomposing the tableau, please call `Widget.decompose()` before extracting the adjacencies")

        if self.io_map is None:
            io_map = POINTER(IOMapType)()
            ptr = POINTER(IOMapType)(io_map)
            lib.widget_get_io_map_api(self.widget, ptr)
            self.io_map = IOMap(self.get_initial_qubits(), io_map)

        return self.io_map


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
        '''
            Decomposes the operation sequence into an algorithmically specific graph (asg)
        '''
        if self.__decomposed:
            raise Exception("Attempted to decompose twice")
        lib.widget_decompose(self.widget)
        self.__decomposed = True


    def load_pandora(self, db_name : str):
        '''
            load_pandora
            Loads gates from a pandora database
        '''
        db_name = c_buffer(db_name.encode('ascii'))
        lib.pandora_n_qubits(db_name)

        lib.pandora_load_db(self.widget, db_name)




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

    def __repr__(self):
        return f"Qubit: {self.src} {list(iter(self))}"

    def __str__(self):
        return self.__repr__()

    def __del__(self):
        lib.widget_destroy_adjacencies(
            POINTER(AdjacencyType)(self.adj)
        )
