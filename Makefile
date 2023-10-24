CC=gcc
YACC=bison
LEX=flex
COFLAGS=-g -O2
CWFLAGS=-Wall -Wextra
CFLAGS=$(CWFLAGS) $(COFLAGS)
all: app

.PHONY:	clean distclean

clean:
	-rm -f *.o lex.yy.c

distclean:
	-rm -f *.o
