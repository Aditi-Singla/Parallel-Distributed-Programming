## Lab 2 - Travelling Salesman Problem using Genetic Algorithms

This assignment involves implementation of a parallel (OpenMP) solution to the NP-Hard problem of traveling salesman (TSP)
using genetic algorithms (GA). Statement for the same is linked [here](./Statement.pdf).


#### Running the code

1. To compile -

    ```bash
    ./scripts/compile.sh
    ```

2. To run - 

    ```bash
    ./scripts/run.sh <input-file> <output-file> --numThreads  <number of threads>
    ```
    

#### Code Description

1. **Parallel Algorithm :**

    The algorithm uses PMX and GX crossover strategies. Initially, a population of n size is initialised. Then, it is sorted and from the first m chromosomes, randomly 2 parents are selected and 2 children are produced, one by PMX and one by GX. This is continued to make a complete population of size n. And now the complete algorithm is repeated for a given number of iterations.

2. **Design Decisions :** 

    - Initial Population Size, n = 3000
    - Number of fittest parents, m = 1000
    - Number of iterations (generations produced) = 1000 

3. **Parallelisation Strategy :**

    The outermost loop has been parallelised to reduce inter-thread dependencies. Each thread has a copy of initial population and the above algorithm is then run on each thread. And a global variable is maintained, which is kept under critical section.

4. **Load Balancing Strategy :**

    Scheduling of threads is dynamic, so each thread gets almost similar workload. The number of iterations on each thread is dynamically balanced.

5. **Results :**

    |   Threads   |      Time     |
    | ----------- | ------------- |
    |      1      |   12.3874 s   |
    |      2      |   9.04693 s   |
    |      4      |   7.55787 s   |
    |      8      |   7.72795 s   |
    |      16     |   7.96021 s   |
    |      32     |   8.67851 s   |
