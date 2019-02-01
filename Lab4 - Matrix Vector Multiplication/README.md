## Lab 4 - Sparse Matrix-Vector Multiplication

This assignment involves implementation of an algorithm to perform matrix-vector multiplication, matrix being sparse and square. MPI, OpenMP and CUDA have been used to handle the problem. Statement for the same is linked [here](./Statement.pdf).

A detailed report for this assignment can be found [here](./Report.pdf).

#### Code Description

1. **Algorithm :**

    - Take in the matrix input in COO format and convert it to CSR format
    - Split the data into multiple MPI processes in the following way: Send to each process a part of the matrix A and the complete vector B. The distribution is explained below
    - Each process does the multiplication on its part of the data, to generate an output vector, which are later collected at one process and combined to get the final answer

2. **Design Decisions :**

    - MPI has been used to implement this distribution of data amongst processes to maximise the parallelism
    - *CSR Scalar Kernel* : This kernel takes one row of the matrix per thread and computes the multiplication of the row with the vector and returns. Different kernels were tried like DIA and ELL Kernel, but CSR kernel has been chosen since it allows variable number of non-zero elements per row, and hence gives more flexibility
    - The given COO format has been converted to CSR format (instead of using COO directly), to exploit the benefits of CSR kernel
    - Some experimentation has been done wrt the grid size and the block size to come up with the optimal values

3. **Parallelisation Strategy :**

    - The scalar kernel for CSR format has been employed here. Multiplication is carried out by one row per CUDA thread approach. These threads run in parallel to yield the output for the corresponding sub-matrix, which are then later merged to yield the final output
    - The data split has been done so as to ensure each thread gets almost an equal number of non-zero data elements as this is likely to yield a better distribution (than row wise split to different processes) and hence parallelisation

Note : Scripts were meant for running on HPC@IITD.