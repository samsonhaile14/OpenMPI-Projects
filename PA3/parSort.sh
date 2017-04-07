#!/bin/bash
#SBATCH -n 6
#SBATCH --mem=5120MB
#SBATCH --time=00:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA3Par
