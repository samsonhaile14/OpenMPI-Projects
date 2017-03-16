#!/bin/bash
#SBATCH -n 32
#SBATCH -N 4
#SBATCH --mem=2048MB
#SBATCH --time=00:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA2DPar
