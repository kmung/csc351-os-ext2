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

# exclude server.cpp and shell.cpp from the list of source files
SRCS := $(filter-out $(SRC_DIR)/server.cpp $(SRC_DIR)/shell.cpp, $(SRCS))

# additional source files for server and shell
SERVER_SRCS = $(SRC_DIR)/server.cpp $(SRC_DIR)/filesystem.cpp $(SRC_DIR)/bitmap.cpp $(SRC_DIR)/disk.cpp
SHELL_SRCS = $(SRC_DIR)/shell.cpp $(SRC_DIR)/filesystem.cpp $(SRC_DIR)/bitmap.cpp $(SRC_DIR)/disk.cpp

# objs
OBJS = $(SRCS:.cpp=.o)

# shell and server main objs
SHELL_MAIN_OBJ = $(SRC_DIR)/shell.o
SERVER_MAIN_OBJ = $(SRC_DIR)/server.o

all: $(SERVER_TARGET) $(SHELL_TARGET)

$(SHELL_TARGET): $(SHELL_MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SHELL_MAIN_OBJ) $(OBJS)

$(SERVER_TARGET): $(SERVER_MAIN_OBJ) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_MAIN_OBJ) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(SHELL_MAIN_OBJ): $(SRC_DIR)/shell.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER_MAIN_OBJ): $(SRC_DIR)/server.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean distclean

clean:
	-rm -f $(SRC_DIR)/*.o

distclean: clean
	-rm -f $(SHELL_TARGET) $(SERVER_TARGET) $(SRC_DIR)/virtual_disk.vhd
