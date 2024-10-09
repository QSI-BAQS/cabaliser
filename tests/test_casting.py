'''
    Tests basic widget logic
'''
import unittest
import ctypes
from cabaliser.gate_constructors import hard_cast  

class CastTest(unittest.TestCase):

    def test_cast(self, val=16.0):
        val = hard_cast(
            ctypes.c_int,
            ctypes.c_float,
            hard_cast(
                ctypes.c_float,
                ctypes.c_int,
                val)
            )
