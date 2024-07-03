SHELL := /bin/bash
CC := gcc 
CFLAGS := -Werror -Wvla -Wall 

TARGET := cab 
 
SRCDIR := src
LIBDIR := lib
BUILDDIR := build
LIBS := -I${LIBDIR} -lm

SRCFILES := $(wildcard ${SRCDIR}/*.c)
OBJFILES := $(patsubst ${SRCDIR}/%.c, ${BUILDDIR}/%.o, ${SRCFILES})

all : ${TARGET} ${BUILDDIR}    

debug: CFLAGS += -DDEBUG -DDEBUG_LEVEL=3
debug: all 



.PHONY: all
.PHONY: clean

${BUILDDIR} :    
	mkdir -p ${BUILDDIR}

${TARGET} : ${BUILDDIR} ${OBJFILES} 
	${CC} ${CFLAGS} -o $@ ${OBJFILES} ${LIBS} 

${BUILDDIR}/%.o : ${SRCDIR}/%.c
	${CC} ${CFLAGS} ${LIBS} -c -o $@ $^

# Clean build directory
clean: 
	rm -rf ${BUILDDIR}
