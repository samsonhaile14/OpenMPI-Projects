#!/bin/bash
#SBATCH -n 1
#SBATCH --mem=2048MB
#SBATCH --time=00:30:00
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA4Seq 5800 5 inp1.txt inp1.txt 1
