In order to run the sequential mandelbrot program, run the commands:  
```
make PA2Seq  
sbatch SeqMandel.sh  
```
In order to run the static parallel mandelbrot program, run the commands:  
```
make PA2SPar  
sbatch SParMandel.sh  
```
In order to run the dynamic parallel mandelbrot program, run the commands:  
```
make PA2DPar
sbatch DParMandel.sh
```
In order to change the number of cores used, go into the SParMandel.sh to change -n to the number of cores used for the static parallel program, and change the -n from DParMandel.sh to change the number of cores used for the dynamic parallel program. Also, to ensure that there are enough boxes to execute the requested number of tasks, change -N to reflect the number of cores requested divided by 8.
