#include <cmath>
#include <cstdlib>
#include <fstream>
#include <stdio.h>
#include <iostream>

using namespace std;

int mpow(int x)
{
	int ans = 1;
	for(int i=0; i<x; i++)
		ans = ans*2;
	return ans;
}

int main(int argc, char *argv[]) {
	
	long n;
	n = atoi(argv[1]);
	long pown = (long)pow(2,n);
	ofstream file;
	string filename;
	filename += "input";
	filename += argv[1];
	filename += ".txt";
	file.open(filename);
	file << mpow(n) <<"\n";
	for(long i = 0; i < pown; i++) {
		file << rand() % 10000000000 << " ";
	}
	file.close();
}