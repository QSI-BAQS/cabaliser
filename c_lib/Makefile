SHELL := /bin/bash
CC := gcc 
OMPFLAG := -fopenmp
CFLAGS := -Werror -Wvla -Wall --warn-no-unused-variable -Wunused-result ${OMPFLAG} 

# Optimisation Flags
ifeq ($(CC),clang)
	CFLAGS += -O3 -ffast-math -fPIC
else
	CFLAGS += -Ofast -fPIC
endif
CFLAGS += -mavx2 -mlzcnt -mbmi2 

TARGET := lib_cabaliser.so
TARGET_FLAGS := -shared -Wl,-soname,${TARGET}  
 
SRCDIR := src
LIBDIR := lib
BUILDDIR := build

LIBS := -I${LIBDIR} -I${LIBDIR}/simd

TESTDIR := tests
TEST_SRCDIR := ${TESTDIR}/src
TEST_LIBDIR := ${TESTDIR}/lib
TEST_LIBS := -I${TEST_LIBDIR}

BENCHMARK_DIR := benchmarks
BENCHMARK_SRCDIR := ${BENCHMARK_DIR}/src

# Source Files
SRCFILES := $(wildcard ${SRCDIR}/*.c)
OBJFILES := $(patsubst ${SRCDIR}/%.c, ${BUILDDIR}/%.o, ${SRCFILES})
COND_OBJFILES := ${OBJFILES} 


SIMD_SRCDIR := ${SRCDIR}/simd
SIMD_SRCFILES := $(wildcard ${SIMD_SRCDIR}/*.c)
SIMD_OBJFILES := $(patsubst ${SIMD_SRCDIR}/%.c, ${BUILDDIR}/simd/%.o, ${SIMD_SRCFILES})

OBJFILES += ${SIMD_OBJFILES}

# Transpose requires disabling some loop mangling
# The loop vectorisation flags don't mesh well with the casts between pointer types
SIMD_TESTS := test_transpose test_rowsum
ifeq ($(CC),clang)
	# TODO: Currently passes all tests at O0, fails python cliffords at 
	# O2
    SIMD_CFLAGS := -march=native -O2
    SIMD_CFLAGS += -mllvm -enable-loopinterchange -fno-slp-vectorize -fno-vectorize
else
	SIMD_CFLAGS := -march=native -O2 -fno-tree-loop-vectorize -fno-peel-loops
	SIMD_CFLAGS += -fgcse-after-reload -fipa-cp-clone -floop-interchange -floop-unroll-and-jam -fpredictive-commoning -fsplit-loops -fsplit-paths -ftree-loop-distribution -ftree-partial-pre -funswitch-loops -fvect-cost-model=dynamic -fversion-loops-for-strides  
endif

# Test Files
TEST_SRCFILES := $(wildcard ${TEST_SRCDIR}/*.c)
TEST_RUNNERS := $(patsubst ${TEST_SRCDIR}/%.c, ${TESTDIR}/%.out, ${TEST_SRCFILES})

# TODO: Relative pathing 
PAULI_TRACKER_CLIB := pauli_tracker_clib
PAULI_TRACKER_DIR := pauli_tracker
PAULI_TRACKER_RUSTLIB_DIR := pauli_tracker
PAULI_TRACKER_LIB := $(shell pwd)/${PAULI_TRACKER_RUSTLIB_DIR}/target/release
PAULI_TRACKER_HEADERS := ${PAULI_TRACKER_DIR}/lib

LINK_LIBS := -L${PAULI_TRACKER_LIB} -Wl,-Bstatic -l${PAULI_TRACKER_CLIB} -Wl,-Bdynamic 

LIBS += -I${PAULI_TRACKER_HEADERS} 

# Benchmark Files
BENCHMARK_SRCFILES := $(wildcard ${BENCHMARK_SRCDIR}/*.c)
BENCHMARK_RUNNERS := $(patsubst ${BENCHMARK_SRCDIR}/%.c, ${BENCHMARK_DIR}/%.out, ${BENCHMARK_SRCFILES})

.PHONY: all
.PHONY: clean
.PHONY: test tests
.PHONY: run_tests 
.PHONY: benchmark benchmarks
.PHONY: ${PAULI_TRACKER}


# Build Rules
all : ${TARGET} ${BUILDDIR}

debug_L3: CFLAGS += -DDEBUG -DDEBUG_LEVEL=3
debug_L3: test 

debug_L2: CFLAGS += -DDEBUG -DDEBUG_LEVEL=2
debug_L2: test 

debug_L1: CFLAGS += -DDEBUG -DDEBUG_LEVEL=1
debug_L1: test 

debug: CFLAGS += -fopt-info -fopt-info-loop -fopt-info-loop-missed -fopt-info-vec -fopt-info-vec-missed
debug: test 

tests: test pauli_tracker
test: all
test : CFLAGS += -g
test: ${BUILDDIR} ${OBJFILES} ${TEST_RUNNERS} ${PANDORA_TEST_RUNNERS} 

run_tests: test 
	$(for i in tests/*.out; do echo $i; ./$i; done)


benchmark: benchmarks 
benchmarks: CFLAGS += ${OMPFLAG} -pg
benchmarks: ${TARGET} ${BUILDDIR} ${OBJFILES} ${BENCHMARK_RUNNERS} 

${BUILDDIR} :
	mkdir -p ${BUILDDIR}
	mkdir -p ${BUILDDIR}/simd


${TARGET} : ${BUILDDIR} ${OBJFILES} ${SIMD_OBJFILES} ${COND_OBJFILES} ${PAULI_TRACKER_LIB}
	${CC} ${CFLAGS} ${TARGET_FLAGS} -o $@ ${COND_OBJFILES} ${SIMD_OBJFILES} ${LINK_LIBS} ${LIBS}  

# Compilation of the transpose operation
# TODO: TRANSPOSE and TRANSPOSE_CFLAGS is currently unset
# 		(switch at definition for clang vs gcc as needed)
${BUILDDIR}/${TRANSPOSE}.o : ${SRCDIR}/${TRANSPOSE}.c
	${CC} ${CFLAGS} ${TRANSPOSE_CFLAGS} ${LIBS} -c -o $@ $^

# Dependent libraries
pauli_tracker: ${PAULI_TRACKER_LIB}

${PAULI_TRACKER_LIB}: 
	cd ${PAULI_TRACKER_DIR}; make all CC=$(CC) # Pass active CC along

${BUILDDIR}/%.o : ${SRCDIR}/%.c
ifeq ($(CC),clang) # Clang errors for unused linker flags
	${CC} $^ ${CFLAGS} ${LIBS} -c -o $@
else
	${CC} $^ ${CFLAGS} ${LINK_LIBS} ${LIBS} -c -o $@
endif

${BUILDDIR}/simd/%.o : ${SRCDIR}/simd/%.c
	${CC} $^ ${CFLAGS} ${SIMD_CFLAGS} ${LIBS} -c -o $@

${TESTDIR}/%.out : ${OBJFILES} ${TEST_SRCDIR}/%.c
	${CC} $^ ${CFLAGS} ${LINK_LIBS} ${TEST_LIBS} ${LIBS} -o $@ 

${TESTDIR}/${TEST_TRANSPOSE}.out : ${OBJFILES} ${TEST_SRCDIR}/${TEST_TRANSPOSE}.c 
	${CC} $^ ${CFLAGS} ${TRANSPOSE_CFLAGS} ${LINK_LIBS} ${LIBS} -o $@ 

${BENCHMARK_DIR}/%.out : ${OBJFILES} ${BENCHMARK_SRCDIR}/%.c 
	${CC} $^ ${CFLAGS} ${LINK_LIBS} ${LIBS} -o $@

obj:
	@echo ${OBJFILES}

# Clean build directory
clean: 
	rm -rf ${BUILDDIR}
	rm -rf ${TESTDIR}/*.out
	rm -rf ${BENCHMARK_DIR}/*.out
	cd ${PAULI_TRACKER_DIR}; make clean
