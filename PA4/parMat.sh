#!/bin/bash
#SBATCH -n 16
#SBATCH -N 2
#SBATCH --mem=2048MB
#SBATCH --time=00:30:00
#SBATCH --output=parc16-1.txt
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA4Par 5280 10 inp0.txt inp0.txt 0 1
