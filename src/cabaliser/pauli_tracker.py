'''
Pauli Tracker Wrapper
'''

from ctypes import POINTER, c_size_t, c_uint32, c_void_p

from cabaliser.utils import deref, INF

from cabaliser.structs import ScheduleDependencyType
from cabaliser.io_array_wrappers import ScheduleDependency

from cabaliser.lib_cabaliser import lib
lib.lib_pauli_n_layers.restype = c_size_t
lib.lib_pauli_n_dependents.restype = c_size_t
lib.lib_pauli_tracker_partial_order_graph.restype = c_void_p
lib.lib_pauli_layer_to_dependent_node.restype = POINTER(ScheduleDependencyType)


class PauliTracker:
    '''
        PauliTracker
        Wrapper for the rustlib pauli tracker object
    '''
    def __init__(self, widget_ptr):
        '''
            PauliTracker
            Wrapper for the rustlib pauli tracker object
        '''
        self.pauli_tracker_ptr = deref(widget_ptr).pauli_tracker

        self.graph_ptr = None
        self.__n_layers = None
        self.measurement_schedule = None
        self.max_qubit = INF  # Truncates unused qubits

    def __call__(self):
        self.schedule()

    def schedule(self, max_qubit=INF):
        '''
            Sets up the measurement schedule
        '''
        self.max_qubit = max_qubit
        self.graph_ptr = self.widget_to_graph()

    def widget_to_graph(self) -> c_void_p:
        '''
            widget_to_graph
            Gets a graph pointer from a widget pointer
        '''
        graph_ptr = lib.lib_pauli_tracker_partial_order_graph(
            self.pauli_tracker_ptr
            )
        return graph_ptr

    @property
    def n_layers(self) -> int:
        '''
            graph_get_n_layers
            Gets the number of layer in the schedule graph
        '''
        if self.__n_layers is None:
            self.__n_layers = int(lib.lib_pauli_n_layers(self.graph_ptr))
        return self.__n_layers

    def get_layer(self, index: int) -> c_void_p:
        '''
            graph_get_layer
            Gets a layer pointer from the graph
            :: index : int :: Layer index
            Returns an opaque pointer to the layer object
        '''
        return lib.lib_pauli_graph_to_layer(self.graph_ptr, c_uint32(index))

    def layer_get_n_dependents(self, layer_ptr: c_void_p) -> int:
        '''
            layer_get_n_dependents
            Gets the number of dependent nodes in a layer
            :: layer_ptr : c_void_p :: Opaque pointer to layer object
            Returns the number of elements in that layer
        '''
        return int(lib.lib_pauli_n_dependents(layer_ptr))

    def layer_get_dependent(self, layer_ptr: c_void_p, index: int) -> ScheduleDependency:
        '''
            layer_get_dependent
            Gets a dependent node from a layer
            :: layer_ptr : ctypes.c_void_p ::
            :: index : int :: Index of dependent
            Returns a wrapped qubit array object
        '''
        qubit_index = int(lib.lib_pauli_dependent_qubit_idx(layer_ptr, index))
        node_ptr = lib.lib_pauli_layer_to_dependent_node(layer_ptr, c_uint32(index))
        node = deref(node_ptr)
        return ScheduleDependency(node.len, qubit_index, node.arr)

    def __iter__(self):
        if self.measurement_schedule is None:
            self.construct_measurement_schedule()
        return iter(self.measurement_schedule)

    def construct_measurement_schedule(self):
        '''
            construct_measurement_schedule
            Constructs a measurement schedule from
            the rust library
        '''
        if self.measurement_schedule is None:
            schedule = []
            for layer_idx in range(self.n_layers):
                layer = self.get_layer(layer_idx)
                schedule.append([])
                n_dependents = self.layer_get_n_dependents(layer)
                for dep_idx in range(n_dependents):
                    dependent = self.layer_get_dependent(layer, dep_idx)
                    if dependent.qubit_index < self.max_qubit:
                        schedule[-1].append(dependent.to_dict())
            self.measurement_schedule = schedule
        return self.measurement_schedule

    def debug_print(self):
        '''
            Call through to the rust print functions for the graph and the tracker
        '''
        lib.lib_pauli_tracker_print(self.pauli_tracker_ptr)
        lib.lib_pauli_tracker_graph_print(self.graph_ptr)
