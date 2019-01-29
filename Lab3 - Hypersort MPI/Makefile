CC = mpic++
CXXFLAGS = -std=c++11 -O2
PFLAGS = -fopenmp

all: hypersort generate

hypersort: 
	$(CC) src/hypersort.cpp $(CXXFLAGS) $(PFLAGS) -o hypersort

generate:
	$(CC) src/generate.cpp $(CXXFLAGS) -o generate	

clean : 
	rm -f hypersort generate