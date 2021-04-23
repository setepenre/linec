CC=g++
CXXFLAGS=-I/usr/include/llvm -std=c++2a -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS

all: bin/linec bin/example

src/parser.cpp: src/parser.y
	bison -d -o $@ $^

src/tokens.cpp: src/tokens.l
	flex -o $@ $^ 

obj/%.o: src/%.cpp
	$(CC) -g -c $(CXXFLAGS) $^ -o $@

bin/linec: obj/parser.o obj/tokens.o obj/node.o obj/environment.o obj/main.o 
	$(CC) -g $^ $(shell llvm-config --ldflags --system-libs --libs core) -o $@

bin/example: bin/linec example/example.lc
	$^ $@

clean:
	rm src/parser.hpp src/parser.cpp src/tokens.cpp obj/*.o bin/*

list-dependencies:
	@flex --version
	@bison --version | head -n 1
	@llc --version | grep 'version'
	@g++ --version | head -n 1
