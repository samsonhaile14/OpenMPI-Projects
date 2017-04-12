//Sequential matrix multiplication
// by Samson Haile

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "mpi.h"

using namespace std;

//main program
int main(int argc, char *argv[])
{

	//variables
	long long int index, jndex, kndex;
	int x,y, max_width, max_height,disp_width,disp_height;
	

	if(argc < 1){
		return 1;
	}
	
	max_width = max_height = atoll(argv[1]);

	//initialization
	MPI_Init(&argc, &argv);	//only used for timer
	
	//Set appropriate number of elements per matrix
		vector<long long int> vTemp(max_width, 0);
		vector< vector<long long int> > matA(max_height,vTemp);
		vector< vector<long long int> > matB(max_height,vTemp);
		vector< vector<long long int> > result(max_height,vTemp);
		vTemp.clear();
		
	//designate matrix values
	for(index = 0; index < max_width; index++){
		for(jndex = 0; jndex < max_height; jndex++){
			matA[index][jndex] = (1 + (random() % 9999));
			matB[index][jndex] = (1 + (random() % 9999));
		}
	}
			
	//compute product
	for(disp_width = max_width; disp_width <= max_width; disp_width += 100){
		disp_height = disp_width;

		//start timer
		double start = MPI_Wtime();

		//matrix multiply
		for(index = 0; index < disp_width; index++){
			for(jndex = 0; jndex < disp_height; jndex++){
				for(kndex = 0; kndex < disp_width; kndex++){
					result[index][jndex] += matA[index][kndex] * matB[kndex][jndex];
				}
			}
		}

		//end timer
		double end = MPI_Wtime();
		
		//calculate elapsed time and output
		printf("%d, %f\n", disp_width, end - start);

		}

	MPI_Finalize();

}

