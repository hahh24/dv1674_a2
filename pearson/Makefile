# Build overview: Sequential and pthread parallel executables share helpers.
CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -g -Wunused -Wall
# -pthread selects pthread support at both compilation and linking below.
THREADFLAGS = -pthread

all: pearson pearson_par verify

# Build the optimized sequential executable.
pearson: pearson.o vector.o dataset.o analysis.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the parallel executable; linking also needs -pthread.
pearson_par: pearson_par.o vector.o dataset.o analysis.o analysis_par.o
	$(CXX) $(CXXFLAGS) $(THREADFLAGS) $^ -o $@

verify: verify.c
	$(CC) $< -o $@

pearson.o: pearson.cpp analysis.hpp dataset.hpp vector.hpp
vector.o: vector.cpp vector.hpp
dataset.o: dataset.cpp dataset.hpp vector.hpp
analysis.o: analysis.cpp analysis.hpp vector.hpp
pearson_par.o: pearson_par.cpp analysis_par.hpp dataset.hpp vector.hpp
analysis_par.o: analysis_par.cpp analysis_par.hpp analysis.hpp vector.hpp

# Compile the parallel translation units with -pthread as well.
pearson_par.o analysis_par.o:
	$(CXX) $(CXXFLAGS) $(THREADFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Remove built executables and object files so the next build starts fresh.
clean:
	rm -f pearson pearson_par verify *.o

.PHONY: all clean