//Sequential matrix multiplication
// by 
// 
//Description: This program multiplies two matrices to obtain a result. The program can be computed over 
//				sub-sizes of the largest sized matrix, for which the maximum size is specified in the batch
//				file used to execute the program executable

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "mpi.h"

using namespace std;

//function prototypes
void transpose(vector< int > &matB, long long int max_width);
void printMat( vector<int> matA, int mat_width);

//main program
int main(int argc, char *argv[])
{

	//variables
	long long int index, jndex, kndex;
	long long int x,y, max_width, max_height,sub_sizes, disp_width,disp_height;	
	bool outputResult = false;

	if(argc <= 2){
		return 1;
	}
	
	if(argc > 3){
		fin.open(argv[4]);
	}
	
	if(argc > 5){
		outputResults = (bool)atoi(argv[5]);
	}
	
	max_width = max_height = atoll(argv[1]);
	sub_sizes = atoll(argv[2]);
	
	//initialization
	MPI_Init(&argc, &argv);

	//check if files are good
	if(fin.good()){
		fin.close();
		fin.open(argv[3]);
		if(fin.good()){
			fin >> max_width;
			max_height = max_width;
		}
	}
	
	//Set appropriate number of elements per matrix
		vector< int > matA(max_height*max_width,0);
		vector< int > matB(max_height*max_width,0);
		vector< long long int > result(max_height*max_width,0);
		
	//designate matrix values
			//random generation input
			if(!fin.good()){
				for(index = 0; index < max_height * max_width; index++){
						matA[index] = (1 + (random() % 9999));
						matB[index] = (1 + (random() % 9999));
				}
			}

			//file input
			else{
				for( index = 0; index < max_width * max_height; index++ ){
						fin >> matA[index];
				}
				
				fin.close();
				fin.open(argv[4]);

				fin >> index;
				for( index = 0; index < max_width * max_height; index++ ){
						fin >> matB[index];
				}
				
				fin.close();

			}
	
	//transpose matB for contiguous access
	transpose(matB, max_width);
			
	//compute product
	for(disp_width = max_width / sub_sizes; disp_width <= max_width; disp_width += max_width / sub_sizes){
		disp_height = disp_width;

		if(outputResult){
			//tests correctness
				printMat(matA,disp_width);
				printMat(matB,disp_width);
		}

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
		if(outputResult){
			for(index = 0; index < disp_height; index++){
				for(jndex = 0; jndex < disp_width; jndex++){
					printf("%lld ", result[index*disp_width + jndex]);
				}
				printf("\n");
			}
		}

		//calculate elapsed time and output
		printf("%lld, %f\n", disp_width, end - start);

		}

	MPI_Finalize();

}

//function implementation

	//transposes matrix to ensure multiplication occurs along contiguous elements
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
	
	//Prints multiplied matrices to console screen
	void printMat( vector<int> matA, int mat_width){
		
		for(int index = 0; index < mat_width; index++){
			for(int jndex = 0; jndex < mat_width; jndex++){
				printf("%d ", matA[index * mat_width + jndex]);
			}
			printf("\n");
		}
		printf("\n\n\n");
	}
