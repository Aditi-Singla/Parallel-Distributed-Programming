# !/bin/sh
### Set the job name
# PBS -N ayush12
### Set the project name, your department dc by default
# PBS -P 
### Request email when job begins and ends
#PBS -m bea
### Specify email address to use for notification.
# PBS -M cs5140281@iitd.ac.in
####
# PBS -l select=32:ncpus=1
### Specify "wallclock time" required for this job, hhh:mm:ss
# PBS -l walltime=30:00:00

#PBS -l software=
# After job starts, must goto working directory. 
# $PBS_O_WORKDIR is the directory from where the job is fired. 
echo "==============================="
echo $PBS_JOBID
cat $PBS_NODEFILE
echo "==============================="
cd $PBS_O_WORKDIR
#job 

module load compiler/cuda/8.0/compilervars
module load compiler/gcc/5.1.0/compilervars
module load mpi/mpich/3.1.4/gcc/mpivars

nvcc -I/home/soft/mpich-3.1.4/include/ -L/home/soft/mpich-3.1.4/lib/ -lmpi sparseMatVector.cu -o main

# mpic++ -o my_exec -std=c++0x -O3 hyperquicksort_2014CS10201.cpp

time mpirun -np 4 ./main input.txt output.txt
#NOTE
# The job line is an example : users need to change it to suit their applications
# The PBS select statement picks n nodes each having m free processors
# OpenMPI needs more options such as $PBS_NODEFILE
