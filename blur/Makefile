# Build overview: Sequential and pthread parallel executables share helpers.
CXX = g++
CXXFLAGS = -std=c++17 -g -Wunused -Wall
# -pthread selects pthread support at both compilation and linking below.
THREADFLAGS = -pthread

all: blur blur_par

# Build the optimized sequential executable.
blur: blur.o matrix.o ppm.o filters.o
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build the parallel executable; linking also needs -pthread.
blur_par: blur_par.o matrix.o ppm.o filters.o filters_par.o
	$(CXX) $(CXXFLAGS) $(THREADFLAGS) $^ -o $@

blur.o: blur.cpp matrix.hpp ppm.hpp filters.hpp
matrix.o: matrix.cpp matrix.hpp ppm.hpp
ppm.o: ppm.cpp ppm.hpp matrix.hpp
filters.o: filters.cpp filters.hpp matrix.hpp ppm.hpp
blur_par.o: blur_par.cpp filters_par.hpp matrix.hpp ppm.hpp
filters_par.o: filters_par.cpp filters_par.hpp filters.hpp matrix.hpp ppm.hpp

# Compile the parallel translation units with -pthread as well.
blur_par.o filters_par.o:
	$(CXX) $(CXXFLAGS) $(THREADFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Remove built executables and object files so the next build starts fresh.
clean:
	rm -f blur blur_par *.o

.PHONY: all clean