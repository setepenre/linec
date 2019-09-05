CC=g++
CXXFLAGS=-I/usr/lib/llvm-8/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

all: bin/linec bin/example

parser.cpp: parser.y
	bison -d -o $@ $^

tokens.cpp: tokens.l
	flex -o $@ $^ 

%.o: %.cpp
	$(CC) -g -c $(CXXFLAGS) $^ -o $@

bin/linec: parser.o tokens.o node.o main.o 
	$(CC) -g $^ $(shell llvm-config --ldflags --system-libs --libs core) -o $@

bin/example: bin/linec example/example.lc
	bin/linec example/example.lc $@

clean:
	rm parser.hpp parser.cpp tokens.cpp *.o bin/*

list-dependencies:
	@flex --version
	@bison --version | head -n 1
	@llc --version | grep 'version'
	@g++ --version | head -n 1
