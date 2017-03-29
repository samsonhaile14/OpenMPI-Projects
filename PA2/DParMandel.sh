#!/bin/bash
#SBATCH -n 32
#SBATCH -N 4
#SBATCH --mem=2048MB
#SBATCH --time=00:05:00

srun PA2DPar
