#TO BE USED WITH LATEX
#TARGET =
#CC = xelatex
#CCFLAGS = -shell-escape
#
#all: $(TARGET).pdf
#
#%.pdf:	%.tex title/* files/*
#	$(CC) $(CCFLAGS) $<
#
#
#.PHONY:	clean all spell
#clean:
#	rm -f $(TARGET).{pdf,log,aux,pyg}
#
#spell:
#	aspell -l el_gr --ignore-case=true -c $(TARGET).tex
#TO BE USED WITH LATEX
TARGET = generate
CC = gcc
CCFLAGS +=
OPT = 2

DEBUG = "n"

ifeq ($(DEBUG),"y")
	CCFLAGS += -D$(TARGET)_DEBUG=1 -D$(TARGET)
	CCFLAGS += -g -O0 -Werror -Wall -Wextra -Wuninitialized
else
	CCFLAGS += -D$(TARGET)_DEBUG=0 -D$(TARGET)
	CCFLAGS += -O$(OPT) -Werror -Wall
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
	rm -f $(OBJ) $(TARGET)
indent:
	astyle --style=linux $(CCFILES)
tags:
	ctags -R *
