## Cabaliser

A graph state compiler written in C.

Most of what you want is in the `c_lib` directory
The Python side of the code provides a thin wrapper around the C code.


## Dependencies: ##
- Should now support both x86 and ARM chipsets
- `rustc` is required to compile the Pauli Tracker library  
- Python should be at least 3.10
  - numpy is required
  - pytest is recommended
- It is assumed that the user is doing their own python environment management.


## Installation ##

In the top level directory:
- make: Should the C library then install cabaliser as a python package in whatever environment is currently active.
- make test: Builds and runs both C and python tests
- make clean: Uninstalls the package and removes the shared object file. 


To build the C library standalone, look at the make file in `c_lib`:
- `make`, this should build the shared object library 
- `make test` will build the tests, which are stored in the `tests` directory.
- `make benchmark` will build some seeded random benchmarks, that are stored in the `benchmarks` directory
- `make paulitracker` will build just the Pauli tracker

To build the Python wrapper:
- `pip install -e .`
Editable installs are currently required as under the new python package management system linking the shared object file makes this a `dynamic' module. 


## Examples ## 
You can find some simple examples in the examples directory.
Cabaliser provides support for naive widgetisation on the Python end, but better performance is generally found if widgets are prepared with a more robust strategy (such as in Pandora).  

## Known Issues ## 

Python pre-3.10 handled class methods, functions and decorator methods differently, as a result Python >= 3.10 is required.

Python's ctypes casting has edge cases that cast objects to `int32`, which may result in segmentation faults when trying to use ctypes pointers with other library functions.

The module is currently flagging as a dynamic module, we need to do some additional work to build to get to a non-editable install. 
