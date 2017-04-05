#!/bin/bash
#SBATCH -n 1
#SBATCH --mem=4096MB
#SBATCH --time=00:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA3Seq 5
