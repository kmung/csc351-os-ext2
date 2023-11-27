CC=g++
COFLAGS=-g -O2
CWFLAGS=-Wall -Wextra
CFLAGS=$(CWFLAGS) $(COFLAGS)

# shell and server names
SHELL_TARGET ?= shell
SERVER_TARGET ?= server

# source code directory
SRC_DIR ?= ./fs

# source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# objs
OBJS = $(SRCS:.cpp=.o)

# shell and server main objs
SHELL_MAIN_OBJ = $(SRC_DIR)/shell.o
SERVER_MAIN_OBJ = $(SRC_DIR)/server.o

all: $(SHELL_TARGET) $(SERVER_TARGET)

$(SHELL_TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SHELL_MAIN_OBJ)

$(SERVER_TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_MAIN_OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:	clean distclean

clean:
	-rm -f $(SRC_DIR)/*.o

distclean:
	-rm -f $(SRC_DIR)/*.o $(SHELL_TARGET) $(SERVER_TARGET)
