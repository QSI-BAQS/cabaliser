SHELL := /bin/bash
CC := gcc 
OMPFLAG := -fopenmp
CFLAGS := -Werror -Wvla -Wall --warn-no-unused-variable ${OMPFLAG}

# Optimisation Flags
CFLAGS += -Ofast -fPIC
CFLAGS += -mavx2 -mlzcnt -mbmi2 

TARGET := lib_cabaliser.so
TARGET_FLAGS := -shared -Wl,-soname,${TARGET}  
 
SRCDIR := src
LIBDIR := lib
BUILDDIR := build

LIBS := -I${LIBDIR}

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

# Transpose requires disabling some loop mangling
# The loop vectorisation flags don't mesh well with the casts between pointer types
TRANSPOSE := simd_transpose
TRANSPOSE_TEST := test_transpose
TRANSPOSE_CFLAGS := -march=native -O2 -fno-tree-loop-vectorize -fno-peel-loops
TRANSPOSE_CFLAGS += -fgcse-after-reload -fipa-cp-clone -floop-interchange -floop-unroll-and-jam -fpredictive-commoning -fsplit-loops -fsplit-paths -ftree-loop-distribution -ftree-partial-pre -funswitch-loops -fvect-cost-model=dynamic -fversion-loops-for-strides  
TRANSPOSE_CFLAGS_CLANG := -march=native -O1

# Test Files
TEST_SRCFILES := $(wildcard ${TEST_SRCDIR}/*.c)
TEST_RUNNERS := $(patsubst ${TEST_SRCDIR}/%.c, ${TESTDIR}/%.out, ${TEST_SRCFILES})

PAULI_TRACKER_DIR :=  pauli_tracker
PAULI_TRACKER_RUSTLIB_DIR := ${PAULI_TRACKER_DIR}/pauli_tracker 
PAULI_TRACKER_LIB := ${PAULI_TRACKER_RUSTLIB_DIR}/target/release/libpauli_tracker.rlib

MBQC_SCHEDULER_DIR := mbqc_scheduling
MBQC_SCHEDULER_RUSTLIB_DIR := ${MBQC_SCHEDULER_DIR}/mbqc_scheduling 
MBQC_SCHEDULER_LIB := ${MBQC_SCHEDULER_RUSTLIB_DIR}/target/release/libmbqc_scheduling.rlib

# Benchmark Files
BENCHMARK_SRCFILES := $(wildcard ${BENCHMARK_SRCDIR}/*.c)
BENCHMARK_RUNNERS := $(patsubst ${BENCHMARK_SRCDIR}/%.c, ${BENCHMARK_DIR}/%.out, ${BENCHMARK_SRCFILES})

.PHONY: all
.PHONY: clean
.PHONY: test tests
.PHONY: benchmark benchmarks
.PHONY: pandora_api
.PHONY: ${PAULI_TRACKER}
.PHONY: ${MBQC_SCHEDULER}


# Build Rules
all : ${TARGET} ${BUILDDIR} 

include pandora_api/Makefile 

pandora : COND_OBJFILES += ${PANDORA_OBJFILES} 
pandora : LIBS += ${PANDORA_LIBS} 
pandora : pandora_api all

debug_L3: CFLAGS += -DDEBUG -DDEBUG_LEVEL=3
debug_L3: test 

debug_L2: CFLAGS += -DDEBUG -DDEBUG_LEVEL=2
debug_L2: test 

debug_L1: CFLAGS += -DDEBUG -DDEBUG_LEVEL=1
debug_L1: test 

debug: CFLAGS += -fopt-info -fopt-info-loop -fopt-info-loop-missed -fopt-info-vec -fopt-info-vec-missed
debug: test 

test : CFLAGS += -g
tests : test pandora_test
test: ${BUILDDIR} ${OBJFILES} ${TEST_RUNNERS} ${PANDORA_TEST_RUNNERS} 


benchmark: benchmarks 
benchmarks: CFLAGS += ${OMPFLAG} -pg
benchmarks: ${BUILDDIR} ${OBJFILES} ${BENCHMARK_RUNNERS} 



${BUILDDIR} :    
	mkdir -p ${BUILDDIR}

${TARGET} : ${BUILDDIR} ${OBJFILES} ${COND_OBJFILES}
	${CC} ${CFLAGS} ${TARGET_FLAGS} -o $@ ${COND_OBJFILES} ${LIBS}  

# Compilation of the transpose operation
${BUILDDIR}/${TRANSPOSE}.o : ${SRCDIR}/${TRANSPOSE}.c
ifeq ($(CC),gcc) # Clang doesn't support some of the optimisation flags
		${CC} ${CFLAGS} ${TRANSPOSE_CFLAGS} ${LIBS} -c -o $@ $^
else
		${CC} ${CFLAGS} ${TRANSPOSE_CFLAGS_CLANG} ${LIBS} -c -o $@ $^
endif

# Dependent libraries
pauli_tracker: ${PAULI_TRACKER_LIB}
mbqc_scheduling: ${MBQC_SCHEDULER_LIB}
${PAULI_TRACKER_LIB}:
	cd ${PAULI_TRACKER_RUSTLIB_DIR}; cargo build --release
${MBQC_SCHEDULER_LIB}:
	cd ${MBQC_SCHEDULER_RUSTLIB_DIR}; cargo build --release


${BUILDDIR}/%.o : ${SRCDIR}/%.c
	${CC} ${CFLAGS} ${LIBS} -c -o $@ $^

${TESTDIR}/%.out : ${OBJFILES} ${TEST_SRCDIR}/%.c
	${CC} ${CFLAGS} ${TEST_LIBS} ${LIBS} -o $@ $^

${TESTDIR}/${TEST_TRANSPOSE}.out : ${OBJFILES} ${TEST_SRCDIR}/${TEST_TRANSPOSE}.c 
	${CC} ${CFLAGS} ${TRANSPOSE_CFLAGS} ${LIBS} -o $@ $^


${BENCHMARK_DIR}/%.out : ${OBJFILES} ${BENCHMARK_SRCDIR}/%.c 
	${CC} ${CFLAGS} ${LIBS} -o $@ $^


obj:
	@echo ${OBJFILES}

# Clean build directory
clean: 
	rm -rf ${BUILDDIR}
	rm -rf ${TESTDIR}/*.out
	rm -rf ${BENCHMARK_DIR}/*.out
	cd ${PAULI_TRACKER_RUSTLIB_DIR}; cargo clean
	cd ${MBQC_SCHEDULER_RUSTLIB_DIR}; cargo clean
