import os
from ctypes import cdll, POINTER

lib_path = os.path.dirname(__file__)

lib = cdll.LoadLibrary(f'{lib_path}/../../c_lib/lib_cabaliser.so')
