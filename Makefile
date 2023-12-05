CC=g++
COFLAGS=-g -O2
CWFLAGS=-Wall -Wextra
CFLAGS=$(CWFLAGS) $(COFLAGS)

# shell and server names
SHELL_TARGET ?= shell
SERVER_TARGET ?= server

# source code directory
SRC_DIR ?= ./fs

# source files for shell
SHELL_SRCS = $(SRC_DIR)/shell.cpp

# source files for server
SERVER_SRCS = $(SRC_DIR)/server.cpp $(SRC_DIR)/filesystem.cpp $(SRC_DIR)/bitmap.cpp $(SRC_DIR)/disk.cpp

# objs for shell
SHELL_OBJS = $(SHELL_SRCS:.cpp=.o)

# objs for server
SERVER_OBJS = $(SERVER_SRCS:.cpp=.o)

all: $(SERVER_TARGET) $(SHELL_TARGET)

$(SHELL_TARGET): $(SHELL_OBJS)
	$(CC) $(CFLAGS) -o $(SHELL_TARGET) $(SHELL_OBJS)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean distclean

clean:
	-rm -f $(SRC_DIR)/*.o

distclean: clean
	-rm -f $(SHELL_TARGET) $(SERVER_TARGET) virtual_disk.vhd
