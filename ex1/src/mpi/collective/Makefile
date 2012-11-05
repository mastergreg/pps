TARGET = main
CC = mpicc
CCFLAGS +=
OPT = 1

ifndef DEBUG
       DEBUG = n
endif

ifeq ($(DEBUG),y)
	CCFLAGS += -D$(TARGET)_DEBUG=1
	CCFLAGS += -g -O0 -Wall -Wextra -Wuninitialized
else
	CCFLAGS += -D$(TARGET)_DEBUG=0
	CCFLAGS += -O$(OPT) -Wall
endif

CCFILES += $(wildcard *.c)
OBJ +=  $(patsubst %.c,%.o,$(CCFILES))
DEPENDS += $(wildcard *.h) 


all: $(TARGET).exec

$(TARGET).exec: $(OBJ) $(DEPENDS)
	$(CC) $(OBJ) -o $(TARGET).exec

%.o:	%.c
	$(CC) -c $(CCFLAGS) $< -o $@


.PHONY:	clean all indent tags
clean:
	rm -f $(OBJ) $(TARGET).exec
indent:
	astyle --style=linux $(CCFILES)
tags:
	ctags -R *
