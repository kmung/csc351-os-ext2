CC=g++
COFLAGS=-g -O2
CWFLAGS=-Wall -Wextra
CFLAGS=$(CWFLAGS) $(COFLAGS)

# shell name
TARGET ?= crash

# source code directory
SRC_DIR ?= ./fs

# source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# objs
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:	clean distclean

clean:
	-rm -f $(SRC_DIR)/*.o

distclean:
	-rm -f $(SRC_DIR)/*.o $(TARGET)
