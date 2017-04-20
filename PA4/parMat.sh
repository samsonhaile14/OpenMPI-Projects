#!/bin/bash
#SBATCH -n 16
#SBATCH -N 2
#SBATCH --mem=2048MB
#SBATCH --time=00:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA4Par 5800 5
