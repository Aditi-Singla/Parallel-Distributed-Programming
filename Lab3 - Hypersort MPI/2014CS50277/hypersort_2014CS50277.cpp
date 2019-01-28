#include <iostream>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <vector>
#include <mpi.h>
#include <stdio.h>
#include <cassert>
using namespace std;

int maxSize;

void input(char filename[], vector<long> &v){
	ifstream in;
	in.open(filename);
	if (!in.is_open()){
		cout << "File not found.\n";
	}
	else{
		int size = 0;
		in >> size;
		for (int i = 0; i < size; i++){
			long a = 0;
			in >> a;
			v.push_back(a);
		}
	}
}

void output(char filename[], vector<long> &list){
	ofstream out(filename, ios::out);
	for (int i = 0; i < list.size(); i++){
		out << list[i] << " ";
	}
}

long quicksort(vector<long> &v){
	sort(v.begin(), v.end());
	if (v.size()%2 == 0)
		return v[(v.size()/2) - 1];
	else
		return v[v.size()/2];
}

long getMid(vector<long> &v){
	if (v.size()%2 == 0)
		return v[(v.size()/2) - 1];
	else
		return v[v.size()/2];
}

void merge(vector<long> &v1, vector<long> &v2, vector<long> &v){
	int i1 = 0;
	int i2 = 0;
	while (i1 < v1.size() && i2 < v2.size()){
		if (v1[i1] <= v2[i2]){
			v.push_back(v1[i1]);
			i1++;
		}
		else{
			v.push_back(v2[i2]);
			i2++;
		}
	}
	while (i1 < v1.size()){
		v.push_back(v1[i1]);
		i1++;
	}
	while (i2 < v2.size()){
		v.push_back(v2[i2]);
		i2++;
	}
	v1.clear();
	v2.clear();
}

void hypersort(char filename1[], char filename[]){
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Get the number of processes
	int totalProcs;
	MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);
	
	// Get the rank of the process
	int procNum;
	MPI_Comm_rank(MPI_COMM_WORLD, &procNum);

	vector<long> v;
	if (procNum == 0){
		vector<long> totallist;
		input(filename1, totallist);
		maxSize = int(totallist.size());
		int sizeSubarray = (totallist.size())/totalProcs;
		for (int i = 0; i < sizeSubarray; i++)
			v.push_back(totallist[i]);
		for (int i = 1; i < totalProcs; i++){
			if (i == totalProcs-1){
				int toSendSize = int(totallist.size()) - (i*sizeSubarray);
				MPI_Send(&toSendSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				if (toSendSize != 0)
					MPI_Send(&totallist[i*sizeSubarray], toSendSize, MPI_LONG, i, 0, MPI_COMM_WORLD);
			}
			else{
				int toSendSize = sizeSubarray;
				MPI_Send(&toSendSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				if (toSendSize != 0)
					MPI_Send(&totallist[i*sizeSubarray], toSendSize, MPI_LONG, i, 0, MPI_COMM_WORLD);
			}
		}
		totallist.clear();
	}
	else{
		int toReceiveSize = 0;
		MPI_Recv(&toReceiveSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		v.resize(toReceiveSize);
		if (toReceiveSize != 0)
			MPI_Recv(&v[0], toReceiveSize, MPI_LONG, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	double start = MPI_Wtime();
	
	long median = quicksort(v);

	vector<long> self;
	vector<long> received;	
	for (int i = 0; i < log2(totalProcs); i++){
		int numPartitions = pow(2,i);
		int sizePartition = totalProcs/numPartitions;
		long median = getMid(v);
		
		if (procNum % sizePartition == 0){
			for (int k = procNum+1; k < procNum+sizePartition; k++){
				MPI_Send(&median, 1, MPI_LONG, k, 0, MPI_COMM_WORLD);
			}
		}
		else{
			MPI_Recv(&median, 1, MPI_LONG, sizePartition * (procNum/sizePartition), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		if (procNum % sizePartition < sizePartition/2){
			// Send right data to its partner
			int cutIndex = 0;
			while (v.size() > 0 && v[cutIndex] <= median){
				self.push_back(v[cutIndex]);
				cutIndex++;
				if (cutIndex == int(v.size()))
					break;
			}
			cutIndex--;
			int size = int(v.size());
			int toSendSize = size-(cutIndex+1);
			MPI_Send(&toSendSize, 1, MPI_INT, (procNum+(sizePartition/2)), 0, MPI_COMM_WORLD);
			int toReceiveSize = 0;
			MPI_Recv(&toReceiveSize, 1, MPI_INT, (procNum+(sizePartition/2)), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			received.resize(toReceiveSize);
			if (toSendSize != 0)
				MPI_Send(&v[cutIndex+1], toSendSize, MPI_LONG, (procNum+(sizePartition/2)), 0, MPI_COMM_WORLD);
			// Receive left data from its partner
			if (toReceiveSize != 0)
				MPI_Recv(&received[0], toReceiveSize, MPI_LONG, (procNum+(sizePartition/2)), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		else{
			// Send right data to its partner
			int cutIndex = int(v.size())-1;
			if (cutIndex != -1){	
				while (v[cutIndex] > median){
					self.push_back(v[cutIndex]);
					cutIndex--;
					if (cutIndex < 0)
						break;
				}
				reverse(self.begin(),self.end());
			}
			int toSendSize = cutIndex+1;
			// cout << "toSendSizeb : " << toSendSize << endl;
			MPI_Send(&toSendSize, 1, MPI_INT, (procNum-(sizePartition/2)), 0, MPI_COMM_WORLD);
			int toReceiveSize = 0;
			MPI_Recv(&toReceiveSize, 1, MPI_INT, (procNum-(sizePartition/2)), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			received.resize(toReceiveSize);
			// Receive left data from its partner
			if (toReceiveSize != 0)
				MPI_Recv(&received[0], toReceiveSize, MPI_LONG, (procNum-(sizePartition/2)), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (toSendSize != 0)
				MPI_Send(&v[0], toSendSize, MPI_LONG, (procNum-(sizePartition/2)), 0, MPI_COMM_WORLD);
		}
		v.clear();
		merge(self, received, v);
	}
	vector<long> retlist;
	if (procNum != 0){
		int toSendSize = v.size();
		MPI_Send(&toSendSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		if (toSendSize != 0)
			MPI_Send(&v[0], toSendSize, MPI_LONG, 0, 0, MPI_COMM_WORLD);
		// cout << "Time taken : " << (MPI_Wtime() - start) << "s" << endl;
	}
	else{
		retlist.resize(maxSize);
		int iterator = 0;
		for (iterator = 0; iterator < v.size(); iterator++){
			retlist[iterator] = v[iterator];
		}
		for (int i = 1; i < totalProcs; i++){
			int toReceiveSize = 0;
			MPI_Recv(&toReceiveSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			received.resize(toReceiveSize);
			if (toReceiveSize != 0)
				MPI_Recv(&retlist[iterator], toReceiveSize, MPI_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			iterator += toReceiveSize;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if (procNum == 0){
		cout << "Time taken : " << (MPI_Wtime() - start) << "s" << endl;
		output(filename, retlist);
	}
	// Finalize the MPI environment.
	MPI_Finalize();
}

int main(int argc, char** argv){
	hypersort(argv[1], argv[2]);
}