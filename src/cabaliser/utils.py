'''
    Misc useful utilities
'''
from ctypes import c_void_p

INF = float('inf')


def deref(obj: c_void_p) -> object:
    '''
        Verbose dereference for pointer objects
    '''
    return obj[0]


class Builder: 
    '''
        Generic class object that emulates a dictionary
    '''
    def __getitem__(self, attr):
        return getattr(self, attr)

    def __setitem__(self, attr, value):
        return setattr(self, attr, value)

    def __in__(self, attr):
        return attr in dir(self)
    
    def __iter__(self):
        iter(self.slots)
