#!/bin/bash
#SBATCH -n 2
#SBATCH -N 2
#SBATCH --mem=2048MB
#SBATCH --time=01:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun proj1 10000
