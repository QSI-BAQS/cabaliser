'''
    Operation Sequence
    Wrapper for sequences of operations
    Exposes these sequences to the C api
'''
from cabaliser.gates import SINGLE_QUBIT_GATES, TWO_QUBIT_GATES, RZ
from cabaliser.operations import (
    OperationType, SingleQubitOperation,
    TwoQubitOperation, RzOperation)


class OperationSequence():
    '''
        Operation sequence object
        This is a wrapper for an array of ctypes
         operations
        :: n_instructions: int :: Maximum number of
         instructions for this sequence
        This object has a pre-allocated maximum number
         of supported operations
    '''
    CONSTRUCTOR_MAP = (
          {i: SingleQubitOperation for i in SINGLE_QUBIT_GATES}
        | {i: TwoQubitOperation for i in TWO_QUBIT_GATES}
        | {RZ: RzOperation})

    def __init__(self, n_instructions: int):
        '''
        Constructor for the operation sequence object
        :: n_instructions: int :: Maximum number of instructions for this sequence
        '''
        self.n_instructions = n_instructions
        self.ops = (OperationType * n_instructions)()
        self.curr_instructions = 0

        self.max_qubit_index = 0
        self.n_rz_operations = 0

    def __getitem__(self, idx: int):
        '''
            __getitem__
            Gets an operation from the array
            :: idx: int :: index to query
            Warning: negative integers will return the value from
            negative addresses, they will NOT index from the end
            of the array
        '''
        return self.ops[idx]

    def __setitem__(self, idx, value: OperationType):
        '''
            __setitem__
            Sets an operation from the array
            :: idx: int :: index to query
            :: value: OperationType :: The operation to write to that element
            Warning: negative integers will write to addresses
            below the address of the array,
            they will NOT index from the end of the array
        '''
        self.ops[idx] = value
        self.max_qubit_index = max(self.max_qubit_index, value.max_qubit_index)

    def __iter__(self):
        '''
            __iter__
            Iterator for the OperationSequence object
            Iterates over the set of currently set operations
        '''
        for i in range(self.curr_instructions):
            yield self.ops[i]

    def append(self, opcode, *args):
        '''
            append
            :: opcode :: Operation code to append
            :: *args :: Arguments to pass to the instruction
            Calls an internal constructor table to build the appropriate operation
        '''
        if self.curr_instructions == self.n_instructions:
            raise IndexError("Exceeded Length of Array")

        # This function acts to append the opcode to the ops array
        self.CONSTRUCTOR_MAP[opcode](self.ops, self.curr_instructions, opcode, *args)

        # Update sequence params
        if opcode == RZ:
            self.sequence_params_rz(*args)
        else:
            self.sequence_params(*args)
        self.curr_instructions += 1

    def _append(self, *operations):
        if self.curr_instructions + len(operations) > self.n_instructions:
            raise IndexError("Exceeded Length of Array")

        for operation in operations:
            self[self.curr_instructions] = operation
            self.curr_instructions += 1

    def __add__(self, other):
        '''
            __add__
            Helper function to make these arrays more list-like
            :: other : OperationSequence :: Another operation sequence
            Returns a new operation sequence containing lhs operations then rhs operations
        '''
        seq = OperationSequence(self.n_instructions + other.n_instructions)
        # This could probably be done with a wrap around memcpy
        for op in self:
            seq._append(op)
        for op in other:
            seq._append(op)

        seq.max_qubit_index = max(self.max_qubit_index, other.max_qubit_index)
        seq.n_rz_operations = self.n_rz_operations + other.n_rz_operations
        return seq

    def __repr__(self):
        return '\n'.join(repr(self.ops[i]) for i in range(self.curr_instructions))

    def __str__(self):
        return self.__repr__()

    def sequence_params_rz(self, *params):
        '''
            Updates parameters for rz operations
            This corresponds to one extra qubit
        '''
        self.n_rz_operations += 1
        self.sequence_params(*params)

    def sequence_params(self, *params):
        '''
            Updates the maximum qubit index in use
        '''
        self.max_qubit_index = max(self.max_qubit_index, *params)
