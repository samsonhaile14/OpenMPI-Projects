#!/bin/bash
#SBATCH -n 1
#SBATCH --mem=2048MB
#SBATCH --time=00:30:00
#SBATCH --output=seqTest.txt
#SBATCH --mail-user=shaile@h1.cse.unr.edu
#SBATCH --mail-type=ALL

srun PA4Seq 4320 10 inp0.txt inp0.txt 0
