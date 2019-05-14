
CPPFLAGS=--std=c++17 -Wall -Wpedantic -Wextra -DGSL_THROW_ON_CONTRACT_VIOLATION

all: bin bin/main

clean:
	rm -f bin/*

bin:
	mkdir -p bin

bin/main: src/main.cpp
	g++ $(CPPFLAGS) -o bin/main src/main.cpp

run: bin/main
	bin/main

# Header file dependencies
bin/main: src/maxclique.hpp
bin/main: ../../include/recursion.hpp ../../include/sense.hpp
