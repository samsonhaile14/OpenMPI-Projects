#!/bin/bash
#SBATCH -n 2
#SBATCH -N 1
#SBATCH --mem=2048MB
#SBATCH --time=00:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA4Par 5800 1 inp1.txt inp1.txt 1
