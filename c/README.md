## ##

## Dependencies ##

- `postgresql-libs, libpqxx` or equivalent packages that provides libqp headers for pandora integration
- Compiler is assumed to be `gcc`, if you are compiling on a Mac you may discover that Apple has helpfully aliased `gcc` to `clang`.
- Various operations assume that your CPU supports `bmi2` and `avx2` instructions along with some collection of atomic operations.

## Installation ##

- `make`, this should build the shared object library 
- `make test` will build the tests, which are stored in the `tests` directory.
- `make benchmark` will build some random benchmarks, that are stored in the `benchmarks` directory

