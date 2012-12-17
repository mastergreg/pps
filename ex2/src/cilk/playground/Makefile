CC = cilkc

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



NPROCS = 4
CCFILES += $(wildcard *.cilk)
OBJ +=  $(patsubst %.cilk,%.o,$(CCFILES))
EXECFILES +=  $(patsubst %.cilk,%.exec,$(CCFILES))
DEPENDS += $(wildcard *.h) 

all: $(EXECFILES)

debug:
	make -B DEBUG=y

%.exec: %.o
	$(CC) $(LDFLAGS) $< -o $@

%.o:	%.cilk $(DEPENDS)
	$(CC) -c $(CCFLAGS) $< -o $@

.PHONY:	clean all indent tags check
clean:
	rm -f $(OBJ) $(EXECFILES)
indent:
	astyle --style=linux $(CCFILES)
tags:
	ctags -R *
check:	$(EXECFILES)
	@echo Running checks
	$(foreach exec,$(EXECFILES),./$(exec) --nproc=$(NPROCS);)
