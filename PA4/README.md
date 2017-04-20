Data file documentation:  
Data Files not included for this submission
___  
Compiling and Running Code:  
In order to compile the code, either the command 'make PA4Seq' or 'make PA4Par' must be run to create an executable for the sequential matrix multiplication or parallel matrix multiplication respectively. After both executables are created, to run the executables, the command 'sbatch seqMat.sh' or sbatch 'parMat.sh' must be run to execute the sequential/parallel sort executables respectively.  
___  
Modifying batch file parameters:  
The batch file executes the executables with command arguments so that different matrix sizes and sub sizes can be accomodated. Each batch file parMat.sh and seqMat.sh takes in 2 additional arguments, referring to max_size and sub_size respectively. max_size represents the largest sized matrix the program uses for matrix multiplication. sub_size denotes the number of smaller iterations the matrix multiplication program executes up to. For instance, executing seqMat.sh with the command 'srun PA4Seq 5000 5' executes matrix multiplication on matrix sizes 1000, 2000, 3000, 4000, and 5000.