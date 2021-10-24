# Source Repo
https://bitbucket.org/etcamargo/parallelsorting/src/master/

# Abstract

High Performance Computing systems rely on parallel algorithms that can take a very long time -- up to several days -- to execute. These systems are expensive in terms of the computational resources required and energy consumption. Thus, after failures occur it is highly desirable to loose as little of the work that has already been done as possible. In this work we present an Algorithm-Based Fault Tolerance (ABFT) strategy that can be applied to make a robust version of any hypercube-based parallel algorithm. Note that we do not assume a physical hypercube: after nodes crash,  fault-free nodes autonomously adapt themselves according to a logical topology called VCube, preserving several logarithmic properties. The proposed strategy guarantees that the algorithm does not halt even after up to N-1 nodes crash, in a system of $N$ nodes. We use parallel sorting as a case study, describing how to make fault-tolerant versions of three classic hypercube-based parallel sorting algorithms: HyperQuickSort, QuickMerge, and Bitonic Sort. The algorithms were implemented in MPI using ULMF to handle faults. Experimental results are presented showing the performance and robustness of the proposed solutions.

# Parallel Algorithms

We implemented fault-tolerant versions of three classic parallel sorting algorithms: HyperQuickSort, QuickMerge, and Bitonic Sort. HyperQuickSort is a parallel version of the well known QuickSort sequential algorithm designed for the hypercube topology. QuickMerge is also a parallel version of QuickSort for hypercubes. The main difference between HyperQuickSort and QuickMerge is on how pivots are chosen and communicated; while in QuickMerge all pivots are computed and broadcast as the algorithm starts to execute, Hyperquicksort requires pivots to be selected and transmitted along the execution rounds. Both these algorithms are classified as data-driven, while the third algorithm we implemented, Bitonic Sort, is data-independent. This classification refers to whether the performance of the algorithm depends on the input (data-driven) or not (data-independent). Data-driven algorithms are usually faster, but can present worst-cases with very low performance for particular input configurations. On the other hand, as the performance of data-independent algorithms does not vary according to the input, they are more predictable. This feature has made Bitonic Sort one of the algorithms most frequently chosen for different settings, such as for GPUs which cannot modify the output location in memory on the basis of input sequence.

The algorithms were implemented in the C language using Open MPI and the ULFM 2.0 library.


# Resultados

We present the experimental results obtained for the three fault-tolerant parallel sorting algorithms in our paper. Each experiment consisted of having each of the algorithms sort 1 billion randomly generated integers. Four scenarios were evaluated. After showing results for a baseline scenario with (1) no faults; three different scenarios varied in terms of the number of nodes that became faulty: (2) a single node; (3) half the node; (4) N-1 nodes. The total number of nodes N varied from 4, 8, 16 up to 32 nodes. Each experiment was repeated 10 times, results presented are averages. Soon our paper will be available here.

