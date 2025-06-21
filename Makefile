UNAME := $(shell uname -s)

ifeq ($(UNAME),Darwin)
    CC = clang++
    CC += -D_XOPEN_SOURCE -Wno-deprecated-declarations
    LIBCPU = libcpu_macos.o
else
    CC = g++
    LIBCPU = libcpu.o
endif

CC += -g -Wall -std=c++17

# Core thread library source files
THREAD_SOURCES = cpu.cpp thread.cpp global.cpp mutex.cpp cv.cpp
THREAD_OBJS = $(THREAD_SOURCES:.cpp=.o)

# Test source files in tests/ directory
TEST_SOURCES = $(wildcard tests/test*.cpp)
TEST_BINS = $(patsubst tests/%.cpp, tests/%, $(TEST_SOURCES))

# Default target
all: libthread.o $(TEST_BINS)

# Build thread library object
libthread.o: $(THREAD_OBJS)
	ld -r -o $@ $(THREAD_OBJS)

# Build each test binary
tests/%: tests/%.cpp libthread.o $(LIBCPU)
	$(CC) -o $@ $^ -ldl -pthread

# Clean all generated files
clean:
	rm -f $(THREAD_OBJS) libthread.o $(TEST_BINS) output/*.out *.dSYM
