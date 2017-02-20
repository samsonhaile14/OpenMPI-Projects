#!/bin/bash
#SBATCH -n 2
#SBATCH --mem=2048MB
#SBATCH --time=00:00:30
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun source/PA1.c
