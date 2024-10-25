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
