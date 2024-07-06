SHELL := /bin/bash
CC := gcc
CFLAGS := -Werror -Wvla -Wall

# Optimisation Flags
CFLAGS += -O3

TARGET := cab
 
SRCDIR := src
LIBDIR := lib
BUILDDIR := build

LIBS := -I${LIBDIR} -lm

TESTDIR := tests
TEST_SRCDIR := ${TESTDIR}/src

# Source Files
SRCFILES := $(wildcard ${SRCDIR}/*.c)
OBJFILES := $(patsubst ${SRCDIR}/%.c, ${BUILDDIR}/%.o, ${SRCFILES})


# Test Files
TEST_SRCFILES := $(wildcard ${TEST_SRCDIR}/*.c)
TEST_RUNNERS := $(patsubst ${TEST_SRCDIR}/%.c, ${TESTDIR}/%.out, ${TEST_SRCFILES})

.PHONY: all
.PHONY: clean
.PHONY: tests

# Build Rules
all : ${TARGET} ${BUILDDIR}    

debug_L3: CFLAGS += -DDEBUG -DDEBUG_LEVEL=3
debug3: all 

debug_L2: CFLAGS += -DDEBUG -DDEBUG_LEVEL=2
debug2: all 

debug_L1: CFLAGS += -DDEBUG -DDEBUG_LEVEL=1
debug1: all 

debug: CFLAGS += -DDEBUG -DDEBUG_LEVEL=0
debug: all 


tests : test
test: CFLAGS += -DDEBUG -DDEBUG_LEVEL=0
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
