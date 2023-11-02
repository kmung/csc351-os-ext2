CC=g++
YACC=bison
LEX=flex
COFLAGS=-g -O2
CWFLAGS=-Wall -Wextra
CFLAGS=$(CWFLAGS) $(COFLAGS)

# source code directory
SRC_DIR ?= ./app

all: app

.PHONY:	clean distclean

clean:
	-rm -f *.o lex.yy.c

distclean:
	-rm -f *.o
