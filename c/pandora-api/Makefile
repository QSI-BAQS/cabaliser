SHELL := /bin/bash
CC := gcc
CFLAGS := -Werror -Wvla -Wall --warn-no-unused-variable ${OMPFLAG}

LIBPQ := /lib/libpq.so

# Optimisation Flags
CFLAGS += -Os -Ofast # We aren't doing floating point arithmetic, so this is fine
CFLAGS += -mavx -mavx2 -mlzcnt 

# Library Flags
# These are here to make sure that optimisations don't ruin the library
CFLAGS += -fkeep-inline-functions -fkeep-static-functions 

TARGET := cab
 
SRCDIR := src
LIBDIR := lib
BUILDDIR := ../build/pandora

LIBS := -I${LIBDIR} -L${LIBPQ} -lpq 

TESTDIR := tests
TEST_SRCDIR := ${TESTDIR}/src



# Source Files
SRCFILES := $(wildcard ${SRCDIR}/*.c)
OBJFILES := $(patsubst ${SRCDIR}/%.c, ${BUILDDIR}/%.o, ${SRCFILES})


# Test Files
TEST_SRCFILES := $(wildcard ${TEST_SRCDIR}/*.c)
TEST_RUNNERS := $(patsubst ${TEST_SRCDIR}/%.c, ${TESTDIR}/%.out, ${TEST_SRCFILES})

# Benchmark Files
BENCHMARK_SRCFILES := $(wildcard ${BENCHMARK_SRCDIR}/*.c)
BENCHMARK_RUNNERS := $(patsubst ${BENCHMARK_SRCDIR}/%.c, ${BENCHMARK_DIR}/%.out, ${BENCHMARK_SRCFILES})


.PHONY: all
.PHONY: clean
.PHONY: test tests


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

tests : test
test: ${BUILDDIR} ${OBJFILES} ${TEST_RUNNERS} 


${BUILDDIR} :    
	mkdir -p ${BUILDDIR}

${TARGET} : ${BUILDDIR} ${OBJFILES} 
	${CC} ${CFLAGS} -o $@ ${OBJFILES} ${LIBS} 

${BUILDDIR}/%.o : ${SRCDIR}/%.c
	${CC} ${CFLAGS} ${LIBS} -c -o $@ $^

${TESTDIR}/%.out : ${OBJFILES} ${TEST_SRCDIR}/%.c 
	${CC} ${CFLAGS} ${LIBS} -o $@ $^


# Clean build directory
clean: 
	rm -rf ${BUILDDIR}
	rm -rf ${TESTDIR}/*.out
