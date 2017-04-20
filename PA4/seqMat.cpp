//Sequential matrix multiplication
// by Samson Haile

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "mpi.h"

using namespace std;

void transpose(vector< int > &matB, long long int max_width);

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
		vector< int > matA(max_height*max_width,0);
		vector< int > matB(max_height*max_width,0);
		vector< long long int > result(max_height*max_width,0);
		
	//designate matrix values
	for(index = 0; index < max_height; index++){
		for(jndex = 0; jndex < max_width; jndex++){
			matA[index*max_width + jndex] = (1 + (random() % 9999));
			matB[index*max_width + jndex] = (1 + (random() % 9999));
		}
	}
	
	//transpose matB for contiguous access
	transpose(matB, max_width);
			
	//compute product
	for(disp_width = max_width / 5; disp_width <= max_width; disp_width += max_width / 5){
		disp_height = disp_width;

		//tests correctness
		for(index = 0; index < disp_height; index++){
			for(jndex = 0; jndex < disp_width; jndex++){
				printf("%lld ", matA[index*disp_width + jndex]);
			}
			printf("\n");
		}
		printf("\n\n\n");

		for(index = 0; index < disp_height; index++){
			for(jndex = 0; jndex < disp_width; jndex++){
				printf("%lld ", matB[index*disp_width + jndex]);
			}
			printf("\n");
		}
		printf("\n\n\n");
		
		//start timer
		double start = MPI_Wtime();

		//matrix multiply
		for(index = 0; index < disp_height; index++){
			for(jndex = 0; jndex < disp_width; jndex++){
				for(kndex = 0; kndex < disp_width; kndex++){
					result[index*disp_width + jndex] += (long long int) matA[index*disp_width + kndex] * (long long int) matB[jndex*disp_width + kndex];
				}
			}
		}
		
		//end timer
		double end = MPI_Wtime();

		//tests correctness
		for(index = 0; index < disp_height; index++){
			for(jndex = 0; jndex < disp_width; jndex++){
				printf("%lld ", result[index*disp_width + jndex]);
			}
			printf("\n");
		}
		
		//calculate elapsed time and output
		printf("%d, %f\n", disp_width, end - start);

		}

	MPI_Finalize();

}

//function
void transpose(vector< int > &matB, long long int max_width){
	
	long long int index, jndex;
	int temp;
	
	for(index = 0; index < max_width;index++){
		for(jndex = index + 1; jndex < max_width; jndex++){
			temp = matB[index * max_width + jndex];
			matB[index * max_width + jndex] = matB[jndex * max_width + index];
			matB[jndex * max_width + index] = temp;
		}
	}
	
}

