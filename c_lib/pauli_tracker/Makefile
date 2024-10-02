MAKEFLAGS += --no-builtin-rules
MAKEFLAGS += --no-builtin-variables


LIBDIR := lib

LIBS := -I${LIBDIR}
RUSTLIB := pauli_tracker_clib
NAME := pauli_tracker   

TARGET := target/${NAME}


.PHONY: all
.PHONY: clean  
.PHONY: example  
.PHONY: ${TARGET} 

all : ${RUSTLIB}

${RUSTLIB}:
	cargo build --release



SRCDIR := example
SRCFILES := $(wildcard ${SRCDIR}/*.c)

CFLAGS := -std=c11 -Wall -Wextra -pedantic -Ofast -mavx2
CC := gcc 
LDFLAGS := -L target/release
LDLIBS := -Wl,-Bstatic -l ${RUSTLIB} -Wl,-Bdynamic -fPIC

# you might need to add the following libraries to the LDLIBS
# -lgcc_s -lutil -lrt -lpthread -lm -ldl -lc

example: ${RUSTLIB} ${TARGET}
	./${TARGET}

$(TARGET): ${SRCFILES} 
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LDLIBS) $(LIBS)


clean :  
	cargo clean
	rm -f ${TARGET}
