//Parallel matrix multiplication
// by 
// 
//Description: This program multiplies two matrices and leaves their results spread over
//				the nodes that compute its results. The program can be computed over sub-sizes
//				of the largest sized matrix, for which the maximum size is specified in the batch file

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <string>
#include "mpi.h"

#define MASTER 0

using namespace std;

//function prototypes
	void transpose(vector< int > &matB, long long int max_width);
	void timedOperation( vector< int > subA, vector< int > &subB, vector< long long int > &subR, int rowRange[],
						 long long int disp_width, int numTasks, int taskid, vector<int> &temp);
	void printMat( vector<int> matA, int mat_width);					 
	void printLLMat( vector<long long int> matA, int mat_width, int mat_height,int taskid);

//main program
int main(int argc, char *argv[])
{

	//variables
	long long int index, jndex, kndex;
	long long int max_width, max_height, sub_sizes, disp_width,disp_height;
	int numTasks, taskid;
	bool outputResults = false;
	ifstream fin;

	MPI_Status status;
	
	int rowRange[2];			//element 0: row/col size
								//element 1: row/col start
	
	
	//argument requirements check and read
	if(argc <= 2){
		return 1;
	}
	
	if(argc > 4){
		fin.open(argv[4]);
	}
	
	if(argc > 6){
		outputResults = (bool)atoi(argv[6]);
	}
	
	max_width = max_height = atoll(argv[1]);
	sub_sizes = atoll(argv[2]);
	
	//initialization
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numTasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

	//check if files are good
	for( index = 0; index < numTasks;index++){
		if(index == taskid){
			if(fin.good()){
				fin.close();
				fin.open(argv[3]);
				if(fin.good()){
					fin >> max_width;
					max_height = max_width;
				}
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
	
	//create and allocate memory for vectors
	vector< int > subA( max_width * ((max_height / numTasks) + 1), 0);
	vector< int > subB( max_width * ((max_height / numTasks) + 1), 0);
	vector< long long int > subR( max_width * ((max_height / numTasks) + 1), 0);
	vector< int > temp(max_width * ((max_height / numTasks) + 1), 0);

	//master operation
	if(taskid == MASTER){
				
		//Set appropriate number of elements per matrix
			vector< int > matA(max_width*max_height,0);
			vector< int > matB(max_width*max_height,0);
						
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
				fin >> index;
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
			
		//transpose matrix B for contiguous access
		transpose(matB, max_width);
		
		//compute product
		for(disp_width = max_width/sub_sizes; disp_width <= max_width; disp_width += max_width / sub_sizes){
			disp_height = disp_width;
			vector<int> datSubA(disp_width * disp_height);
			vector<int> datSubB(disp_width * disp_height);
						
			//copy main array to data sub array for contiguous access
			for( index = 0; index < disp_height; index++){
				copy(matA.begin() + (index * max_width), matA.begin() + (index * max_width) + disp_width, datSubA.begin() + disp_width * index );
				copy(matB.begin() + (index * max_width), matB.begin() + (index * max_width) + disp_width, datSubB.begin() + disp_width * index );
			}

			if(outputResults){
			//for testing correctness (prints the matrices that are multiplied to console)
				printMat(datSubA,disp_width);
				printMat(datSubB,disp_width);
			}
			
			int rowDivTasks = disp_height/numTasks;
			int rowModTasks = disp_height%numTasks;
			long long int pos = 0;
			rowRange[1] = pos;
			
			//distribute rows to each process
			for(index = 1; index < numTasks; index++){
				//determine number of rows given to process
				rowRange[0] = rowDivTasks;
				if(index <rowModTasks){
					rowRange[0]++;
				}
				
				//send matrix rows and sizes to each process
				MPI_Send(&rowRange[0], 2, MPI_INT, index, 10, MPI_COMM_WORLD);
				MPI_Send(&datSubA[pos], rowRange[0] * disp_width, MPI_INT, index, 11, MPI_COMM_WORLD);
				MPI_Send(&datSubB[pos], rowRange[0] * disp_width, MPI_INT, index, 12, MPI_COMM_WORLD);
				
				//increment position in matrices
				pos += rowRange[0] * disp_width;
				rowRange[1] += rowRange[0];
				
			}
			
			//implicitly assign master node rows to process (use matrix from matA/B[pos] onwards)
				//determine number of rows given to process
				rowRange[0] = rowDivTasks;
				if(MASTER <rowModTasks){
					rowRange[0]++;
				}
				copy( datSubA.begin() + pos, datSubA.begin() + pos + rowRange[0] * disp_width, subA.begin() );
				copy( datSubB.begin() + pos, datSubB.begin() + pos + rowRange[0] * disp_width, subB.begin() );

			//Ensure all calculations happen at the same time
			MPI_Barrier(MPI_COMM_WORLD);				

			//start timer
			double start = MPI_Wtime();
								
			//perform timed operation
			timedOperation( subA, subB, subR, rowRange, disp_width, numTasks, taskid, temp);
				
			//end timer with barrier to ensure timing is accurate
			MPI_Barrier(MPI_COMM_WORLD);
			double end = MPI_Wtime();

			if(outputResults){
			//for testing correctness (prints multiplication result to file)			
				for(index = 1; index < numTasks; index++){
					if(index == taskid)
						printLLMat(subR,disp_width, rowRange[0],taskid);
					MPI_Barrier(MPI_COMM_WORLD);
				}
				if(MASTER == taskid)
					printLLMat(subR,disp_width, rowRange[0],taskid);

				MPI_Barrier(MPI_COMM_WORLD);
			
			}

			//calculate elapsed time and output
			printf("%d, %lld, %f\n", numTasks, disp_width, end - start);

		}
	}
	
	//slave node operation
	else{
		for(disp_width = max_width/sub_sizes; disp_width <= max_width; disp_width += max_width / sub_sizes){
			disp_height = disp_width;
			
			//receive matrices
			MPI_Recv(&rowRange[0], 2, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
			MPI_Recv(&subA[0], rowRange[0] * disp_width, MPI_INT, 0, 11, MPI_COMM_WORLD, &status);
			MPI_Recv(&subB[0], rowRange[0] * disp_width, MPI_INT, 0, 12, MPI_COMM_WORLD, &status);			
			
			//Ensure all calculations happen at the same time
			MPI_Barrier(MPI_COMM_WORLD);
			
			//perform timed operation
			timedOperation( subA, subB, subR, rowRange, disp_width, numTasks, taskid, temp);
			
			//Barrier to ensure timing is accurate
			MPI_Barrier(MPI_COMM_WORLD);

/*			//for testing correctness (prints multiplication result)
				for(index = 1; index < numTasks; index++){
					if(index == taskid)
						printLLMat(subR,disp_width, rowRange[0],taskid);
					MPI_Barrier(MPI_COMM_WORLD);
				}
				if(MASTER == taskid)
					printLLMat(subR,disp_width, rowRange[0],taskid);

				MPI_Barrier(MPI_COMM_WORLD);
*/			
		}
	}

	MPI_Finalize();
	return 0;
}


//function implementations

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

	//Performs the timed portion of the matrix multiplication
	void timedOperation( vector< int > subA, vector< int > &subB, vector< long long int > &subR, int rowRange[],
						 long long int disp_width, int numTasks, int taskid, vector<int> &temp){
		
		int index,jndex,kndex;
		MPI_Status status;
		
		int colRange[] = {rowRange[0], rowRange[1]};	//element 0 : column size
														//element 1 : column start
		
		//Perform timed operation
		for( int tIndex = 0; tIndex < numTasks; tIndex++){
			
			//Multiply Matrices (storing results in subR)
			for(index = 0; index < rowRange[0]; index++){
				for(jndex = 0; jndex < colRange[0]; jndex++){
					subR[((index) * disp_width) + (jndex+colRange[1])] = 0;
					for(kndex = 0; kndex < disp_width; kndex++){
						subR[((index) * disp_width) + (jndex+colRange[1])] += 
							(long long int) subA[index * disp_width + kndex] * (long long int) subB[jndex * disp_width + kndex];
					}
				}
			}
			
			//Send matrix B rows to next process and receive from process
			copy( subB.begin(), subB.begin() + colRange[0] * disp_width, temp.begin() );

			int tempRange[] = {colRange[0], colRange[1]};
			
			//Pass around columns of matrix B
			for(index = 0; index < numTasks; index++){
				
				if( index == taskid){
					MPI_Send(&tempRange[0], 2, MPI_INT, (index + 1) % numTasks, 13, MPI_COMM_WORLD);
					MPI_Send(&temp[0], tempRange[0] * disp_width, MPI_INT, (index + 1) % numTasks, 14, MPI_COMM_WORLD);
				}
				else if( ((index + 1) % numTasks) == taskid){
					MPI_Recv(&colRange[0], 2, MPI_INT, index, 13, MPI_COMM_WORLD, &status);
					MPI_Recv(&subB[0], colRange[0] * disp_width, MPI_INT, index, 14, MPI_COMM_WORLD, &status);						
				}
				
				MPI_Barrier(MPI_COMM_WORLD);
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

	//prints result matrix pieces to multiple files
	void printLLMat( vector<long long int> matA, int mat_width, int mat_height, int taskid){
		string fName = "ans";
		fName += ('0' + taskid);
		fName += ".dat";
		fstream fout(fName.c_str(), fstream::out);
		for(int index = 0; index < mat_height; index++){
			for(int jndex = 0; jndex < mat_width; jndex++){
				fout << matA[index * mat_width + jndex] << ' ';
			}
			fout << '\n';
		}
		fout.close();
	}
