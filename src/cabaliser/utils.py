'''
    Misc useful utilities
'''
from ctypes import c_void_p

INF = float('inf')

class void_p(c_void_p):
    def __format__(self, *args, **kwargs):
        return super().__format__(*args, **kwargs)
    def __repr__(self, *args, **kwargs):
        return super().__repr__(*args, **kwargs)
    # Subclass (see https://stackoverflow.com/questions/17840144/why-does-setting-ctypes-dll-function-restype-c-void-p-return-long)


def deref(obj: c_void_p) -> object:
    '''
        Verbose dereference for pointer objects
    '''
    return obj[0]
