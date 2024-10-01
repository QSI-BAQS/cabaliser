## Cabaliser

(Name is pending review and new suggestions are more than welcome)

Most of what you want is in the `c` directory
The Python side of the code provides a thin wrapper around the C code 



## Dependencies: ##

- Compiler is assumed to be `gcc`, if you are compiling on a Mac you may discover that Apple has helpfully aliased `gcc` to `clang`.
- Various operations assume that your CPU supports `bmi2` and `avx2` instructions along with some collection of atomic operations.
- `postgresql-libs, libpqxx` or equivalent packages that provides libqp headers for pandora integration

## Installation ##

- `make`, this should build the shared object library 
- `make test` will build the tests, which are stored in the `tests` directory.
- `make benchmark` will build some random benchmarks, that are stored in the `benchmarks` directory


To compile the Pandora integration you will need the appropriate package that provides the `libpq-fe.h` headers for your system.
On arch this is `postgresql-libs`.
