#include <omp.h> 
#include <vector>
#include <math.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <openssl/md5.h>

using namespace std;

#define SIZE 1024*1024*16


vector<unsigned long> arr;
int num_threads;
int n;
int totalSum;
int nThreads;

//// INPUT OUTPUT SECTION

void input(char filename[]){
	ifstream in;
	in.open(filename);
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

void output(char filename[], vector<unsigned long> ans){
	ofstream out(filename, ios::out);
	for (int j=1; j<n; j++){
		out << ans[j] << " ";
	}
	out << totalSum;
	out.close();

	////// Code for MD5 taken from Stack Overflow  ////////////////

	FILE* outptr = fopen(filename,"r");
	unsigned char c[MD5_DIGEST_LENGTH];
	int bytes;
    unsigned char data[1024];
	MD5_CTX mdContext;
	MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, outptr)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);

	///////////////////////////////////////////////////////////////

	cout << "Threads: " << num_threads << endl;
	cout << "Md5-sum: ";
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
		printf("%02x", c[i]);
	}
	cout << "\n";
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
	if (argc >= 3){
		num_threads = pow(2,(int)(log2(atoi(argv[3]))));
	}
	else
		num_threads = 8;

	char* infile = argv[1];
	char* outfile = argv[2];
	// double start = omp_get_wtime();
	
	input(infile);

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
	
	output(outfile, arr);
	// double end2 = omp_get_wtime();
	// cout << "Time taken to print : " << (end2 - end) << endl;

	return 0;
}