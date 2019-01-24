CC = g++
CXXFLAGS = -std=c++11 -O2 -lssl -lcrypto
PFLAGS = -fopenmp -pthread -Wall

all :
	$(CC) src/par-prefix.cpp $(CXXFLAGS) $(PFLAGS) -o par

clean : 
	rm -f par