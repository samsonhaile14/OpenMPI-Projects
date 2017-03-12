PA2Seq: proj2Seq.c
	mpicc -lpmi -o PA2Seq proj2Seq.c

PA2DPar: proj2DynPar.c
	mpicc -lpmi -o PA2DPar proj2SPar.c

PA2SPar: proj2StatPar.c
	mpicc -lpmi -o PA2SPar proj2DPar.c

clean:
	$(RM) PA2Seq PA2DPar PA2SPar
