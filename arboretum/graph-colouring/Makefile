###############################################################
## Basic makefile template.
## Uses google libraries to build test and benchmark suites.
###############################################################

######################## USER CONF ############################

# Define the objects to build (algorithm.cpp -> obj/algorithm.o)
modules = Graph AlgorithmGraphColorNodeVector
# Compiler flags.
common_cppflags = -O3 -Wall -Wextra -pedantic -fPIC --std=c++17 -pthread
cppflags = $(common_cppflags) -DGSL_THROW_ON_CONTRACT_VIOLATION
opt_cppflags = $(common_cppflags) -DNDEBUG
prf_cppflags = $(common_cppflags) -DNDEBUG -g -fno-inline
# Any external libraries to link (gtest/gbench included below).
libs =

######################## AUTO CONF ############################

objects = $(addprefix obj/, $(addsuffix .o, $(modules)))
opt_objects = $(addprefix obj/, $(addsuffix .opt.o, $(modules)))
prf_objects = $(addprefix obj/, $(addsuffix .prf.o, $(modules)))

all: bin obj bin/run

clean:
	@echo " -> Cleaning bin and obj directories"
	@rm -f obj/*.o
	@rm -f bin/*

bin:
	@mkdir -p bin

obj:
	@mkdir -p obj

depend:
	@g++ -MM src/*.cpp | sed -E "s|(\w+)\.o|obj/\1\.o|"
	@g++ -MM src/*.cpp | sed -E "s|(\w+)\.o|obj/\1\.opt\.o|"
	@g++ -MM src/*.cpp | sed -E "s|(\w+)\.o|obj/\1\.prf\.o|"

bin/run: $(opt_objects) obj/main.opt.o
	@echo " -> Linking main binary"
	@g++ -o bin/run $(opt_objects) obj/main.opt.o $(libs)

# Object files.
obj/%.o: src/%.cpp
	@echo " -> Compiling $< in debug mode"
	@g++ $(cppflags) -o $@ $< -c

obj/%.opt.o: src/%.cpp
	@echo " -> Compiling $< in release mode"
	@g++ $(opt_cppflags) -o $@ $< -c

obj/%.prf.o: src/%.cpp
	@echo " -> Compiling $< in profile mode"
	@g++ $(prf_cppflags) -o $@ $< -c
