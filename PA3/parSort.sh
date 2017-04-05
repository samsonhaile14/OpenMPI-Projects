#!/bin/bash
#SBATCH -n 5
#SBATCH --mem=2048MB
#SBATCH --time=00:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA3Par set1.dat
