'''
    Helper class
    Duck typed Python class that can be used to instantiate a Widget-like object from a JSON object 
    The primary use case of this is that the de-JSONed object forces keys as strings
    While the native widget uses integers for some fields 
'''

from functools import partial

class WidgetFromJson():
    '''
        Thin proxy of a widget object from a json object
        This is mostly ducktyped to act like a regular widget
    '''

    def __init__(self, widget_json):
        self.n_qubits = widget_json['n_qubits']
        self.state_nodes = widget_json['statenodes']
        self.adjacencies = self.__key_to_int(widget_json['adjacencies'])
        self.local_cliffords =  widget_json['local_cliffords']
        self.consumption_schedule = list(
                map(list, map(partial(map, self.__key_to_int), widget_json['consumptionschedule']))
            )
        self.measurement_tags = widget_json['measurement_tags']
        self.pauli_corrections = list(map(self.__key_to_int, widget_json['paulicorrections']))
        self.outputnodes = widget_json['outputnodes']
        self.time = widget_json['time']
        self.space = widget_json['space']

    @staticmethod
    def __key_to_int(obj: dict) -> dict:
        '''
            Json formatting dumps keys to strings for dictionaries
            This remaps them to integers
        '''
        remap = {} 
        for key, value in obj.items():
            remap[int(key)] = value
        return remap
   
    def get_input_nodes(self): 
        '''
            Get ordered input indices
        '''
        return self.state_nodes

    def get_n_qubits(self) -> int:
        return self.n_qubits

    def get_local_cliffords() -> list: 
        return self.local_cliffords

    def get_consumption_schedule() -> list:
        return self.consumption_schedule

    def get_measurement_tags(self) -> list:
        return self.measurement_tags

    def get_adjacencies(self):
        return self.adjacencies

    def  get_pauli_correction(self):
        return self.get_pauli_corrections

