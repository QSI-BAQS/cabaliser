## Cabaliser

(Name is pending review and new suggestions are more than welcome)

Most of what you want is in the `c` directory

The Python api can be found in `c/python_api`, on the TODO list is to repackage that into a distutils build for the C library, and then to expose the api as a Python module. 


## Dependencies:

To compile the Pandora integration you will need the appropriate package that provides the `libpq-fe.h` headers for your system.
On arch this is `postgresql-libs`.
