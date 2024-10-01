'''
    Wrapper objects around QubitArray 
    Typing is only for verbosity and any potential extensibility that might be required 
'''
from cabaliser.qubit_array import QubitArray

class MeasurementTags(QubitArray):
    '''
        Ordered array of RZ measurement tags
    '''

class LocalCliffords(QubitArray):
    '''
        Ordered array of local clifford operations
    '''

class IOMap(QubitArray):
    '''
        Ordered array of map of input qubits to output qubits
    '''
