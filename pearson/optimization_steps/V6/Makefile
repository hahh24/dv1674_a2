CXX = g++
CXXFLAGS = -std=c++17 -g -Wunused -Wall
all: pearson
pearson: pearson.o vector.o dataset.o analysis.o
	$(CXX) $(CXXFLAGS) $^ -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f pearson *.o
.PHONY: all clean
