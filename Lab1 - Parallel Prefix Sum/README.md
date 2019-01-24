## Lab 1 - Parallel Prefix Sum

This assignment involves implementation of a prefix-sum algorithm using Pthreads. Statement for the same is linked [here](./Statement.pdf).

#### Code Description

1. **I/O :**

    The input for array size n and the elements of the array, is taken from an input file and stored into a vector of type unsigned long. Maximum number of threads k, allowed can be taken from the command line, which is optional, default is set to 8 threads. In-place prefix sum is calculated and the output (elements of the array) is then printed into an output file. This file is then read to calculate the MD5-sum of the array (code for which is taken from 'Stack Overflow') and 'Maximum No. of threads used in the algorithm', 'Time taken to calculate' & 'MD5-Sum for the output array' are printed on the terminal.

2. **Parallelization Strategy & Algorithm :** 

    Multithreading has been used to find the prefix sum. Given the number of threads as k, the vector is divided into k blocks of size n/k. Here we need the number of threads to be a power of 2. So, if the value of k read from the command line is not a power to 2, k is reset to the nearest exponent of 2, less than that value. This ensures that at any time in every block of array, there are 2^x (for some x) elements. 
    We keep adding alternate values to make a balance tree, by adding every second value, the fourth and so on, till we reach the root. This is called "Up-sweep path". A similar "Down-sweep path" is followed to get the final vector, where the first element is 0, and the rest are the actual values.  The last value is stored separately.

3. **Load-balancing Strategy :**

    Since everytime, the array is divided evenly, with each block being an exponent of 2, at every step, we get proper balanced tree and hence, data parallelism.

4. **Time Complexity :**

    The time complexity is **O(2n/k)**, where each, down and up-sweep blocks need **O(n/k)**. There will be **O(log n)** levels in total.

5. **Data Structures :**

    The array of all the elements is stored as a vector, while the threads and thread arguments are stored in arrays.


#### Running the code

1. To compile -

    ```bash
    ./scripts/compile.sh
    ```

2. To run - 

    ```bash
    ./scripts/run.sh <input-file> <output-file> --numThreads  <number of threads>
    ```
    Note: Number of threads is an optional argument, default being 8. It is rounded down to the nearest power of 2.