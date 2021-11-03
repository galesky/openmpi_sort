# openmpi_sort
Open MPI and Fault Tolerance article

# running
$ mpicc main.c -lm && mpirun -np 4 ./a.out 100000 100000
# Useful links

* Overview of Bitonic sorting in OpenMP, MPI and CUDA
https://people.cs.rutgers.edu/~venugopa/parallel_summer2012/bitonic_overview.html

* OpenMPI Bitonic Sort
https://github.com/moloch--/Bitonic-Sort

* OpenMPI Bitonic Sort using UFLM (referenced by Edson's work)
https://bitbucket.org/WorstOne/parallel-bitonic-sort-ulfm-crash/src/master/