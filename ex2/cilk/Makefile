TARGET = main
CC = cilkc
CCFLAGS += -DUSE_MPI

ifndef DEBUG
       DEBUG = n
endif

ifeq ($(DEBUG),y)
	CCFLAGS += -D$(TARGET)_DEBUG=1
	CCFLAGS += -g -O0 -Wall -Wextra -Wuninitialized
	LDFLAGS += -lefence
else
	CCFLAGS += -D$(TARGET)_DEBUG=0
	CCFLAGS += -Wall
	CCFLAGS+= -march=native -O3 -fexpensive-optimizations -funroll-loops \
		 -fmove-loop-invariants -fprefetch-loop-arrays -ftree-loop-optimize \
		 -ftree-vect-loop-version -ftree-vectorize
endif



CCFILES += $(wildcard *.cilk)
OBJ +=  $(patsubst %.cilk,%.o,$(CCFILES))
DEPENDS += $(wildcard *.h) 

all: $(TARGET).exec

$(TARGET).exec: $(OBJ) $(DEPENDS)
	$(CC) $(LDFLAGS) $(OBJ) -o $(TARGET).exec

%.o:	%.cilk
	$(CC) -c $(CCFLAGS) $< -o $@

.PHONY:	clean all indent tags
clean:
	rm -f $(OBJ) $(TARGET).exec
indent:
	astyle --style=linux $(CCFILES)
tags:
	ctags -R *
