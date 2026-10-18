CXX = g++
CXXFLAGS = -std=c++17 -g -Wunused -Wall
all: blur
blur: blur.o matrix.o ppm.o filters.o
	$(CXX) $(CXXFLAGS) $^ -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f blur *.o
.PHONY: all clean
