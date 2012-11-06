TARGET = main
CC = gcc
CCFLAGS +=
OPT = 2

ifndef DEBUG
       DEBUG = n
endif

ifeq ($(DEBUG),y)
	CCFLAGS += -D$(TARGET)_DEBUG=1
	CCFLAGS += -g -O0 -Werror -Wall -Wextra -Wuninitialized
	LDFLAGS += -lefence
else
	CCFLAGS += -D$(TARGET)_DEBUG=0
	CCFLAGS += -O$(OPT) -Werror -Wall
endif

CCFILES += $(wildcard *.c)
OBJ +=  $(patsubst %.c,%.o,$(CCFILES))
DEPENDS += $(wildcard *.h) 


all: $(TARGET).exec

$(TARGET).exec: $(OBJ) $(DEPENDS)
	$(CC) $(LDFLAGS) $(OBJ) -o $(TARGET).exec

%.o:	%.c
	$(CC) -c $(LDFLAGS) $(CCFLAGS) $< -o $@


.PHONY:	clean all indent tags
clean:
	rm -f $(OBJ) $(TARGET)
indent:
	astyle --style=linux $(CCFILES)
tags:
	ctags -R *
