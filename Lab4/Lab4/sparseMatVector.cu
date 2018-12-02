#include <iostream>
#include <fstream>
#include <vector>
#include <mpi.h>
#include <climits>
#include <stdio.h>
#include <string>
#include <algorithm>
using namespace std;

#define max 100000
#define SIZE 1024*1024*4

string inputfile;

void input(char filename[], vector<int> &row, vector<int> &col, vector<int> &data, vector<int> &v, int &dimension){
	string temp;
	int i,j;
	ifstream in;
	in.open(filename);

	if (!in.is_open()){
		cout << "File not found.\n";
	}
	else{
		// Read "Name: Sample_Input" , useless
		in >> temp;
		in >> inputfile;
		
		// Read "Dimension: n"
		in >> temp;
		in >> dimension;
		
		// Read "A"
		in >> temp;
		
		in >> temp;
		char ch11[max];
		int k = 0;
		while (temp.compare("B") != 0){
			i = 0;
			j = 0;
			while (temp[i]!='\0'){
				ch11[j] = temp[i];
				i++;
				j++;
			}
			ch11[j] = '\0';
			if (k==0)
				row.push_back(atoi(ch11));
			if (k==1)
				col.push_back(atoi(ch11));
			if (k==2)
				data.push_back(atoi(ch11));
			k = (k+1)%3;
			in >> temp;
		}
		for (i = 0; i < dimension; i++){
			in >> j;
			v.push_back(j);
		}
	}
}

__global__ void spmv_csr_scalar_kernel(const int num_rows, const int *ptr, const int *indices, const int *data, const int *x, int *y){
	int row = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < num_rows){
		float dot = 0;
		int row_start = ptr [ row ];
		int row_end = ptr [ row +1];
		for (int j = row_start; j < row_end; j++)
			dot += data[j]*x[indices[j]];
		y[row] += dot ;
	}
}

void sparseMatMul(char filename1[], char filename[]){
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Get the number of processes
	int totalProcs;
	MPI_Comm_size(MPI_COMM_WORLD, &totalProcs);
	
	// Get the rank of the process
	int procNum;
	MPI_Comm_rank(MPI_COMM_WORLD, &procNum);

	vector<int> r,c,d,v;
	int rBegin = 0;
	int rEnd = 0;
	int dimension;
	vector<int> vec;

	if (procNum == 0){
		vector<int> rows,cols,data1;
		input(filename1,rows,cols,data1,vec,dimension);

		int indicesize = (rows.size())/totalProcs;
		for (int i = 0; i < indicesize; i++){
			r.push_back(rows[i]);
			c.push_back(cols[i]);
			d.push_back(data1[i]);
		}
		for (int i = 1; i < totalProcs; i++){
			if (i == totalProcs-1){
				int toSendSize = int(rows.size()) - (i*indicesize);
				MPI_Send(&toSendSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				if (toSendSize != 0){
					MPI_Send(&rows[i*indicesize], toSendSize, MPI_INT, i, 0, MPI_COMM_WORLD);
					MPI_Send(&cols[i*indicesize], toSendSize, MPI_INT, i, 0, MPI_COMM_WORLD);
					MPI_Send(&data1[i*indicesize], toSendSize, MPI_INT, i, 0, MPI_COMM_WORLD);
				}
			}
			else{
				int toSendSize = indicesize;
				MPI_Send(&toSendSize, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				if (toSendSize != 0){
					MPI_Send(&rows[i*indicesize], toSendSize, MPI_INT, i, 0, MPI_COMM_WORLD);
					MPI_Send(&cols[i*indicesize], toSendSize, MPI_INT, i, 0, MPI_COMM_WORLD);
					MPI_Send(&data1[i*indicesize], toSendSize, MPI_INT, i, 0, MPI_COMM_WORLD);
				}

			}
			MPI_Send(&dimension, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			if (dimension != 0){
				MPI_Send(&vec[0], dimension, MPI_INT, i, 0, MPI_COMM_WORLD);
			}
		}
		rows.clear();
		cols.clear();
		data1.clear();
	}
	else{
		int toReceiveSize = 0;
		MPI_Recv(&toReceiveSize, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		r.resize(toReceiveSize);
		c.resize(toReceiveSize);
		d.resize(toReceiveSize);
		if (toReceiveSize != 0){
			MPI_Recv(&r[0], toReceiveSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&c[0], toReceiveSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&d[0], toReceiveSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		MPI_Recv(&dimension, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		vec.resize(dimension);
		if (dimension != 0){
			MPI_Recv(&vec[0], dimension, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}
	int indicesize = r.size();
	rBegin = r[0];
	rEnd = r[indicesize-1];
	int ptrsize = rEnd - rBegin + 2;
	
	int* ptr = (int*)malloc(ptrsize*sizeof(int));
	int* indices = (int*)malloc(indicesize*sizeof(int));
	int* data = (int*)malloc(indicesize*sizeof(int));
	int* x = (int*)malloc(dimension*sizeof(int));
	int* y = (int*)malloc((ptrsize-1)*sizeof(int));

	int init = rBegin-1;
	int count = 0;
	int pointer = 0;
	for (int i = 0; i < indicesize; i++){
		indices[i] = c[i];
		data[i] = d[i];
		if (r[i] != init){
			for (int j = 0; j < r[i]-init; j++){
				ptr[pointer] = count;
				pointer ++;
			}
			init = r[i];
		}
		count++;
	}
	ptr[pointer] = count;
	for (int i = 0; i < dimension; i++){
		x[i] = vec[i];
	}
	MPI_Barrier(MPI_COMM_WORLD);
	
	unsigned int threadsPerBlock = 128;
	unsigned int totalBlocks = (SIZE+(threadsPerBlock-1))/threadsPerBlock;

	// if (procNum == 3){
	// 	for (int i = 0; i < ptrsize; i++){
	// 		cout << ptr[i] << " ";
	// 	}
	// 	cout << endl;
	// 	for (int i = 0; i < indicesize; i++){
	// 		cout << indices[i] << " " << data[i] << endl;
	// 	}
	// 	for (int i = 0; i < dimension; i++){
	// 		cout << x[i] << " " ;
	// 	}
	// 	cout << endl;
	// }
	int *cptr, *cindices, *cdata, *cx, *cy;
	cudaMalloc((void**)&cptr,ptrsize*sizeof(int));
	cudaMalloc((void**)&cindices,indicesize*sizeof(int));
	cudaMalloc((void**)&cdata,indicesize*sizeof(int));
	cudaMalloc((void**)&cx,dimension*sizeof(int));
	cudaMalloc((void**)&cy,(ptrsize-1)*sizeof(int));
	
	cudaMemcpy(cptr,ptr,ptrsize*sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(cindices,indices,indicesize*sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(cdata,data,indicesize*sizeof(int),cudaMemcpyHostToDevice);
	cudaMemcpy(cx,x,dimension*sizeof(int),cudaMemcpyHostToDevice);
	
	cudaMemset(cy,0,(ptrsize-1)*sizeof(int));

	spmv_csr_scalar_kernel<<<totalBlocks, threadsPerBlock>>>(ptrsize-1,cptr,cindices,cdata,cx,cy);

	cudaMemcpy(y,cy,(ptrsize-1)*sizeof(int),cudaMemcpyDeviceToHost);

	///////////// Cuda se merge the vectors ////////////////////
	// int* y = (int*)malloc((ptrsize-1) * sizeof(int));
	if (procNum == 3)
		for (int i = 0; i < ptrsize-1; i++){
			cout << y[i] << " ";
		}
	if (procNum != 0){
		MPI_Send(&rBegin, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(&rEnd, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		if ((rEnd - rBegin + 1) != 0)
			MPI_Send(&y[0], (rEnd - rBegin + 1), MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
	else{
		ofstream out(filename, ios::out);
		int j = 0;
		if (rBegin != 0){
			for (; j < rBegin; j++){
				out << 0 << endl;
			}
		}
		for (; j < rEnd; j++){
			out << y[j] << endl;
		}
		int last = y[rEnd];
		int size1,size2;
		for (int i = 1; i < totalProcs; i++){
			MPI_Recv(&size1, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&size2, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			vector<int> v;
			v.resize(size2 - size1 + 1);
			if ((size2 - size1 + 1) != 0)
				MPI_Recv(&v[0], size2-size1+1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if (size1 == rEnd){
				if (size1 == size2)
					last += v[0];
				else{
					out << v[0]+last << endl;
					for (j = 1; j < v.size()-1; j++){
						out << v[j] << endl;
					}
					last = v[v.size()-1];
					rEnd = size2;
				}
			}
			else{
				out << last << endl;
				for (j = rEnd+1; j < size1; j++){
					out << 0 << endl;
				}
				if (size1 == size2){
					last = v[0];
					rEnd = size2;
				}
				else{
					out << v[0] << endl;
					for (j = 1; j < v.size()-2; j++){
						out << v[j] << endl;
					}
					last = v[v.size()-1];
					rEnd = size2;
				}
			}		
		}
		out << last << endl;
		for (int i = rEnd+1; i < dimension; i++){
			out << 0 << endl;
		}
		out.close();
	}
	// Finalize the MPI environment.
	MPI_Finalize();
}

int main(int argc, char** argv){
	sparseMatMul(argv[1], argv[2]);
}
