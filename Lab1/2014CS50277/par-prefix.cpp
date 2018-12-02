#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h> 
#include <math.h>
#include <pthread.h>
#include <vector>
#include <fstream>
#include <openssl/md5.h>

using namespace std;

#define SIZE 1024*1024*16


vector<unsigned long> arr;
int num_threads;
int n;
int totalSum;
int nThreads;

//// INPUT OUTPUT SECTION

void input(){
	ifstream in;
	in.open("input.txt");
	if (!in.is_open()){
		printf("File not found.\n");
	}
	else{
		in >> n;
		for (int i=0; i<n; i++){
			int temp = 0;
			in >> temp;
			arr.push_back(temp);
		}
	}
}

void output(vector<unsigned long> ans, double time){
	ofstream out("output1.txt", ios::out);
	for (int j=1; j<n; j++){
		out << ans[j] << " ";
	}
	out << totalSum;
	out.close();

	////// Code for MD5 taken from Stack Overflow  ////////////////

	FILE* outptr = fopen("output1.txt","r");
	unsigned char c[MD5_DIGEST_LENGTH];
	int bytes;
    unsigned char data[1024];
	MD5_CTX mdContext;
	MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, outptr)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);

	///////////////////////////////////////////////////////////////

	ofstream out1("output.txt", ios::out);
	out1 << "Threads: " << num_threads << endl;
	out1 << "Time: " << time << endl;
	out1 << "Md5-sum: ";
	out1.close();
	FILE* outptr1 = fopen("output.txt","a");
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		fprintf(outptr1, "%02x", c[i]);
	}
	fprintf(outptr1, "\n");
}

//// ALGORITHM FUNCTIONS

struct pthreadArgs{
	int threadId;
	int level;	///starting from 0 to logn-1
};

void *upSweep(void* pthreadargs)
{
	struct pthreadArgs *data = (struct pthreadArgs*)pthreadargs;
	int n0 = nThreads;
	int id0 = data->threadId;
	int l0 = data->level;
	int a = pow(2,l0);
	int b = (n/n0)*id0;
	for (int i=0; i<(n/n0)-1; i += 2*a){
		arr[b + i + 2*a - 1] +=  arr[b + i + a - 1];
	}
	pthread_exit(NULL);
}	

void *downSweep(void* pthreadargs)
{
	struct pthreadArgs *data = (struct pthreadArgs*)pthreadargs;
	int n0 = nThreads;
	int id0 = data->threadId;
	int l0 = data->level;
	int a = pow(2,l0);
	int b = (n/n0)*id0;
	for (int i=0; i<(n/n0)-1; i += 2*a){
		int t = arr[b + i + a-1];
		arr[b + i + a - 1] =  arr[b + i + 2*a -1];
		arr[b + i + 2*a - 1] += t;
	}
	pthread_exit(NULL);
}	

//// MAIN FUNCTION

int main(int argc, char * argv[]){
	if (argc >= 2){
		num_threads = pow(2,(int)(log2(atoi(argv[1]))));
	}
	else
		num_threads = 8;

	// double start = omp_get_wtime();
	
	input();

	double end1 = omp_get_wtime();
	// cout << "Time taken to read: " << (end1 - start) << endl;
	
	for (int i = 0; i<(int)log2(n); i++){
		///Number of levels
		nThreads = min(num_threads,n/(int)(pow(2,(i+1))));
		int rc;
		pthread_t threads[nThreads];
		struct pthreadArgs threadArgs[nThreads];

		for (int j=0; j<nThreads; j++){
			threadArgs[j].threadId = j;
			threadArgs[j].level = i;
			rc  = pthread_create(&threads[j], NULL, upSweep, (void*) &threadArgs[j]);
			if (rc){
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		}
		for (int j = 0; j<nThreads; j++){
			pthread_join(threads[j], NULL);
		}
		
		// cout << "Level " << i << " : ";
		// for (int j = 0; j < n; j++){
		// 	cout << arr[j] << " " ;
		// }
		// cout << "\n";
	}

	totalSum = arr[n-1];
	arr[n-1] = 0;

	for (int i = (int)log2(n)-1; i>=0; i--){
		///Number of levels
		nThreads = min(num_threads,n/(int)(pow(2,(i+1))));
		int rc;
		pthread_t threads[nThreads];
		struct pthreadArgs threadArgs[nThreads];

		for (int j=0; j<nThreads; j++){
			threadArgs[j].threadId = j;
			threadArgs[j].level = i;
			rc  = pthread_create(&threads[j], NULL, downSweep, (void*) &threadArgs[j]);
			if (rc){
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		}
		for (int j = 0; j<nThreads; j++){
			pthread_join(threads[j], NULL);
		}
		
		// cout << "Level " << i << " : ";
		// for (int j = 0; j < n; j++){
		// 	cout << arr[j] << " " ;
		// }
		// cout << "\n";
	}

	double end = omp_get_wtime();
	cout << "Time taken to calculate: " << (end - end1) << endl;
	
	output(arr, (end-end1));
	// double end2 = omp_get_wtime();
	// cout << "Time taken to print : " << (end2 - end) << endl;

	return 0;
}