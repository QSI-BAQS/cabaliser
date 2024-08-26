PANDORA_SHELL := /bin/bash
PANDORA_CC := gcc
PANDORA_CFLAGS := -Werror -Wvla -Wall --warn-no-unused-variable ${OMPFLAG}

PANDORA_DIR := pandora_api
LIBPQ := /lib/libpq.so

# Optimisation Flags
PANDORA_CFLAGS += -Os -Ofast # We aren't doing floating point arithmetic, so this is fine
PANDORA_CFLAGS += -mavx -mavx2 -mlzcnt 

# Library Flags
# These are here to make sure that optimisations don't ruin the library
PANDORA_CFLAGS += -fkeep-inline-functions -fkeep-static-functions 

PANDORA_TARGET := pandora_api
 
PANDORA_SRCDIR := ${PANDORA_DIR}/src
PANDORA_LIBDIR := ${PANDORA_DIR}/lib
PANDORA_BUILDDIR := ${BUILDDIR}/${PANDORA_DIR}


PANDORA_LIBS := -I${PANDORA_LIBDIR} -I${LIBDIR} -L${LIBPQ} -lpq ${LIB_BENCHQ}   

PANDORA_TESTDIR := tests
PANDORA_TEST_SRCDIR := ${TESTDIR}/src

# Source Files
PANDORA_SRCFILES := $(wildcard ${PANDORA_SRCDIR}/*.c)
PANDORA_OBJFILES := $(patsubst ${PANDORA_SRCDIR}/%.c, ${PANDORA_BUILDDIR}/%.o, ${PANDORA_SRCFILES})

# Test Files
PANDORA_TEST_SRCFILES := $(wildcard ${PANDORA_TEST_SRCDIR}/*.c)
PANDORA_TEST_RUNNERS := $(patsubst ${PANDORA_TEST_SRCDIR}/%.c, ${PANDORA_TESTDIR}/%.out, ${PANDORA_TEST_SRCFILES})

.PHONY: pandora_all
.PHONY: pandora_tests

# Build Rules
pandora_api : ${PANDORA_TARGET} ${BUILDDIR}    

pandora_tests : test
pandora_test: ${BUILDDIR} ${PANDORA_OBJFILES} ${TEST_RUNNERS} 

pandora_echo: 
	@echo ${PANDORA_SRCDIR} 
	@ls ${PANDORA_SRCDIR}/ 


${PANDORA_BUILDDIR} : ${BUILDDIR}    
	mkdir -p ${PANDORA_BUILDDIR}


${PANDORA_TARGET} : ${PANDORA_BUILDDIR} ${PANDORA_OBJFILES} 
	${PANDORA_CC} ${PANDORA_CFLAGS} -o $@ ${PANDORA_OBJFILES} ${PANDORA_LIBS} 

${PANDORA_BUILDDIR}/%.o : ${PANDORA_SRCDIR}/%.c
	${PANDORA_CC} ${PANDORA_CFLAGS} ${PANDORA_LIBS} -c -o $@ $^

${TESTDIR}/%.out : ${OBJFILES} ${TEST_SRCDIR}/%.c 
	${PANDORA_CC} ${PANDORA_CFLAGS} ${PANDORA_LIBS} -o $@ $^

