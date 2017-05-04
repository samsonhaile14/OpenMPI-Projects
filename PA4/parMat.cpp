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
#include <cmath>

#define MASTER 0

using namespace std;

//function prototypes
	//program init
		int init_program( int argc, char *argv[], int &numTasks, int &taskid, bool &outputResults, bool& doCannon, long long int &max_width, 
							long long int &max_height, long long int &sub_sizes, ifstream &fin);
							
		void loadData( vector<int> &matA, vector<int> &matB, long long int max_width,
							long long int max_height, ifstream &fin, char *argv[]);

		void transpose( vector< int > &matB, long long int max_width);
		
	void timedOperation( vector< int > subA, vector< int > &subB, vector< long long int > &subR, int rowRange[],
							long long int disp_width, int numTasks, int taskid, vector<int> &temp);
	void printMat( vector<int> matA, int mat_width);					 
	void printLLMat( vector<long long int> matA, int mat_width, int mat_height,int taskid);

	//alternative to cannon methods
		void alt_allocMem(	vector<int> &subA, vector<int> &subB, vector<long long int> &subR, vector<int> &temp,
							long long int max_width, long long int max_height, int numTasks );

	//cannon methods
		int cannon_allocMem( vector<int> &subA, vector<int> &subB, vector<long long int> &subR, vector<int> &temp,
								long long int max_width, int &squareProc, int numTasks, int taskid, int& subDim );
		
		void cannon_sendSubMatrix( long long int meshStart, int disp_width, long long int numRows, long long int squareProc, vector< int > srcMat,
								int destProc, vector<int> &tempMem, int tag);
								
		void Shift(vector<int> &sdMat, int rowNum, int taskid, int squareProc, int numRows, bool vert);
		
		void cannon_mult( vector<int> &subA, vector<int> &subB, vector<long long int> &subR, int squareProc, int numTasks, int taskid, 
								int numRows, int disp_width );

	
//main program
int main(int argc, char *argv[])
{

	//variables
		//standard variables
			long long int index, jndex, kndex;
			long long int max_width, max_height, sub_sizes, disp_width,disp_height;
			int numTasks, taskid;
			bool outputResults = false;
			bool doCannon = false;
			ifstream fin;
			
		//process personal vectors
			vector< int > subA;
			vector< int > subB;
			vector< long long int > subR;
			vector< int > temp;
	
		//cannon personal variables
			int squareProc = 1;
			int subDim = 1;
			int numRows = 1;

		//misc
			MPI_Status status;
			
			int rowRange[2];			//element 0: row/col size
										//element 1: row/col start
	
	//Perform startup operations and validation (terminate if insufficient arguments given)
		if( init_program(argc, argv, numTasks, taskid, outputResults, doCannon, max_width, max_height, sub_sizes, fin) ){
			return 1;
		}
	
	//allocate memory for vectors	
		//memory allocation for cannon
			if(doCannon){
				cannon_allocMem( subA, subB, subR, temp, max_width, squareProc, numTasks, taskid, subDim );
			}

		//memory allocation for alternative to cannon
			else{
				alt_allocMem(	subA, subB, subR, temp, max_width, max_height, numTasks );
			}

	//master operation
		if(taskid == MASTER){
					
			//Set appropriate number of elements per matrix
				vector< int > matA(max_width*max_height,0);
				vector< int > matB(max_width*max_height,0);
							
			//designate matrix values (either file or random data)
				loadData( matA, matB, max_width, max_height, fin, argv);
				
			//transpose matrix B for contiguous access
			if(!doCannon){
				transpose(matB, max_width);
			}
				
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

					//for testing correctness (prints the matrices that are multiplied to console)						
						if(outputResults){
							printMat(datSubA,disp_width);
							printMat(datSubB,disp_width);
						}

					//initial data distribution
						//distribute cells to each process via cannon
							if(doCannon){
								
								long long int pos = 0;
								numRows = disp_width / squareProc;
																
								//send data to all other processes
									//copy master's work
										for(index = 0; index < numRows; index++){
											
											copy(datSubA.begin() + (index * (disp_width)), 
													datSubA.begin() +  ((index) * (disp_width)) + numRows, 
													subA.begin() + index * numRows );
													
											copy(datSubB.begin() + (index * (disp_width)), 
													datSubB.begin() +  ((index) * (disp_width)) + numRows, 
													subB.begin() + index * numRows );

										}
										
									//send remaining work
										for(index = 1; index < numTasks; index++){

											cannon_sendSubMatrix( (index / squareProc) * (numRows * disp_width) + (index % squareProc) * numRows,
																	disp_width, numRows, squareProc, datSubA, index, temp, 10);
											cannon_sendSubMatrix( (index / squareProc) * (numRows * disp_width) + (index % squareProc) * numRows,
																	disp_width, numRows, squareProc, datSubB, index, temp, 11);									
										}
									
								//transpose now for contiguous access
									transpose(subB,numRows);
									
								//reorganize mesh to ensure each task holds the proper submatrices
									//perform shifts on A and B 
										for(index = 1; index < squareProc; index++){
											for(jndex = 0; jndex < index; jndex++){
												Shift(subA, index, taskid, squareProc, numRows, false);
												Shift(subB, index, taskid, squareProc, numRows, true);
											}
										}
								
							}
					
						//distribute rows to each process via alternative to cannon							
							else{
								int rowDivTasks = disp_height/numTasks;
								int rowModTasks = disp_height%numTasks;
								long long int pos = 0;
								rowRange[1] = pos;
								
								//assign node rows to each process
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
									rowRange[0] = rowDivTasks;
									if(MASTER <rowModTasks){
										rowRange[0]++;
									}
									copy( datSubA.begin() + pos, datSubA.begin() + pos + rowRange[0] * disp_width, subA.begin() );
									copy( datSubB.begin() + pos, datSubB.begin() + pos + rowRange[0] * disp_width, subB.begin() );
							}								

					//Ensure all calculations happen at the same time
						MPI_Barrier(MPI_COMM_WORLD);				

					//start timer
						double start = MPI_Wtime();
										
					//perform timed operation
						//cannon multiplication
							if(doCannon){
								cannon_mult( subA, subB, subR, squareProc, numTasks, taskid, numRows, disp_width );							
							}

						//alternative multiplication
							else{
								timedOperation( subA, subB, subR, rowRange, disp_width, numTasks, taskid, temp);
							}	
					
					//end timer with barrier to ensure timing is accurate
						MPI_Barrier(MPI_COMM_WORLD);
						double end = MPI_Wtime();

					if(doCannon){
						rowRange[0] = disp_width/squareProc;
					}						
						
					if(outputResults){
						//for testing correctness (prints multiplication result to file)			
						for(index = 1; index < numTasks; index++){
							MPI_Barrier(MPI_COMM_WORLD);
						}
						if(MASTER == taskid)
							printLLMat(subR,disp_width/squareProc, rowRange[0],taskid);

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
					//receive cannon algorithm sized matrices
						if(doCannon){
							long long int pos = 0;
							numRows = disp_width / squareProc;				

							//receive data
								MPI_Recv(&subA[0], numRows * numRows, MPI_INT, MASTER, 10, MPI_COMM_WORLD, &status );
								MPI_Recv(&subB[0], numRows * numRows, MPI_INT, MASTER, 11, MPI_COMM_WORLD, &status );					

							//transpose now for contiguous access
								transpose(subB,numRows);								
								
							//reorganize mesh to ensure each task holds the proper submatrices
								//perform horizontal shifts on A and B (B can be shifted horizontally since the send was transposed)
									for(index = 1; index < squareProc; index++){
										for(jndex = 0; jndex < index; jndex++){
											Shift(subA, index, taskid, squareProc, numRows, false);
											Shift(subB, index, taskid, squareProc, numRows, true);
										}
									}
			
						}
					
					//receive matrices for alternative to cannon algorithm
						else{
							MPI_Recv(&rowRange[0], 2, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
							MPI_Recv(&subA[0], rowRange[0] * disp_width, MPI_INT, 0, 11, MPI_COMM_WORLD, &status);
							MPI_Recv(&subB[0], rowRange[0] * disp_width, MPI_INT, 0, 12, MPI_COMM_WORLD, &status);			
						}
				
				//Ensure all calculations happen at the same time
					MPI_Barrier(MPI_COMM_WORLD);
				
				//perform timed operation
				if(doCannon){
					cannon_mult( subA, subB, subR, squareProc, numTasks, taskid, 
								numRows, disp_width );
				}
				
				else{
					timedOperation( subA, subB, subR, rowRange, disp_width, numTasks, taskid, temp);
				}
				
				//Barrier to ensure timing is accurate
				MPI_Barrier(MPI_COMM_WORLD);
				
				if(doCannon){
					rowRange[0] = disp_width/squareProc;
				}
				
				if(outputResults){
				//for testing correctness (prints multiplication result to file)			
					for(index = 1; index < numTasks; index++){
						if(index == taskid)
							printLLMat(subR,disp_width/squareProc, rowRange[0],taskid);
					MPI_Barrier(MPI_COMM_WORLD);
					}

					MPI_Barrier(MPI_COMM_WORLD);
				
				}
			}
		}

	//terminate
		MPI_Finalize();
		return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//function implementations

	//General Initialization functions
		//startup validation
			int init_program(int argc, char *argv[], int &numTasks, int &taskid, bool &outputResults, bool &doCannon, long long int &max_width, 
								long long int &max_height, long long int &sub_sizes, ifstream &fin){
									
				int index;					
									
				//argument requirements check and read
					if(argc <= 2){
						return 1;
					}
					
					if(argc > 3){
						fin.open(argv[4]);
					}
					
					if(argc > 5){
						outputResults = (bool)atoi(argv[5]);
					}
					
					if(argc > 6){
						doCannon = (bool)atoi(argv[6]);
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
							//if file read is not good at any point, no file input will be done
							if(fin.good()){
								fin.close();
								fin.open(argv[3]);
								if(fin.good()){
									fin >> max_width;
									max_height = max_width;
								}
								if(taskid != MASTER)
									fin.close();
							}
						}
						MPI_Barrier(MPI_COMM_WORLD);
					}
					
				return 0;
			}
			
		//loads data
			void loadData(vector<int> &matA, vector<int> &matB, long long int max_width,
							long long int max_height, ifstream &fin, char *argv[]){
				int index;

				//random generation input (default input)
					if(!fin.good()){
						for(index = 0; index < max_height * max_width; index++){
								matA[index] = (1 + (random() % 99));
								matB[index] = (1 + (random() % 99));
						}
					}

				//file input (implies both input files can be read)
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
				
			}
		
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
							subA[index * disp_width + kndex] * subB[jndex * disp_width + kndex];
					}
				}
			}
			
			//Send matrix B rows to next process and receive from process
			copy( subB.begin(), subB.begin() + colRange[0] * disp_width, temp.begin() );

			int tempRange[] = {colRange[0], colRange[1]};
			
			//Pass around columns of matrix B
			for(index = 0; index < numTasks; index++){
				
				if( index == taskid && numTasks != 1){
					MPI_Send(&tempRange[0], 2, MPI_INT, (index + 1) % numTasks, 13, MPI_COMM_WORLD);
					MPI_Send(&temp[0], tempRange[0] * disp_width, MPI_INT, (index + 1) % numTasks, 14, MPI_COMM_WORLD);
				}
				else if( ((index + 1) % numTasks) == taskid && numTasks != 1){
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
	
	//alternative methods (to cannon)
		//allocate memory for algorithm
			void alt_allocMem(	vector<int> &subA, vector<int> &subB, vector<long long int> &subR, vector<int> &temp,
								long long int max_width, long long int max_height, int numTasks ){
				subA.resize( max_width * ((max_height / numTasks) + 1), 0);
				subB.resize( max_width * ((max_height / numTasks) + 1), 0);
				subR.resize( max_width * ((max_height / numTasks) + 1), 0);
				temp.resize( max_width * ((max_height / numTasks) + 1), 0);				
				}
		
	//cannon methods
		//allocate memory for algorithm
			int cannon_allocMem(	vector<int> &subA, vector<int> &subB, vector<long long int> &subR, vector<int> &temp,
								long long int max_width, int &squareProc, int numTasks, int taskid, int& subDim ){
				squareProc = sqrt(numTasks);
				
				if(squareProc * squareProc != numTasks){
					if(taskid == MASTER)
						printf("Must use a perfect square number of processes\n");
					
					MPI_Finalize();
					return 1;
				}
				
				if(max_width % squareProc != 0){
					if(taskid == MASTER)
						printf("Dimension of matrix must be evenly divisible by square root of # of processes\n");
					
					MPI_Finalize();
					return 1;
				}
				
				subDim = max_width / squareProc;
				
				subA.resize( subDim * subDim );
				subB.resize( subDim * subDim );
				subR.resize( subDim * subDim );
				temp.resize( subDim * subDim );
			}
		
		//initializes process memory by spreading data across processes
			void cannon_sendSubMatrix(long long int meshStart, int disp_width, long long int numRows, long long int squareProc, vector< int > srcMat, 
									int destProc, vector<int> &tempMem, int tag){
				
				long long int index;
				
				//create contiguous data array so memory is contiguous
					for(index = 0; index < numRows; index++){
						copy(srcMat.begin() + meshStart + (index * (disp_width)), 
							srcMat.begin() +  meshStart + ((index) * (disp_width)) + numRows, 
							tempMem.begin() + index * numRows );						
					}
				
				//send contiguous array to specified process
					MPI_Send(&tempMem[0], numRows * numRows, MPI_INT, destProc, tag, MPI_COMM_WORLD);

				
			}
		
		//shifts row
			void Shift(vector<int> &sdMat, int rowNum, int taskid, int squareProc, int numRows, bool vert){
				int index;
				int procNum;
				int addend;
				
				MPI_Status status;

				//Determines horizontal offset of swap
				if(!vert){
					addend = 1;
				}
				else{
					addend = squareProc;
				}
					
				//row or column shift
					for(index = 0; index < (squareProc-1);index++){
						//specifies number process in iteration
						if(!vert){
							procNum = ((rowNum * squareProc) + index);						
						}
						else{
							procNum = ((index) * squareProc) + rowNum;
						}
							
						//swaps matrices between paired processes
							if(  procNum == taskid ){
								MPI_Sendrecv_replace( &sdMat[0], numRows * numRows, MPI_INT,procNum + addend, 
									10,procNum + addend, 10, MPI_COMM_WORLD, &status );
							}
							
							else if( (procNum + addend) == taskid){
								MPI_Sendrecv_replace( &sdMat[0], numRows * numRows, MPI_INT,procNum, 
									10,procNum, 10, MPI_COMM_WORLD, &status );
							}
						
						//Wait for completion of swap
							MPI_Barrier(MPI_COMM_WORLD);
					}
			}

			void cannon_mult( vector<int> &subA, vector<int> &subB, vector<long long int> &subR, int squareProc, int numTasks, int taskid, 
								int numRows, int disp_width ){
				
				
				int iter,index,kndex,jndex;
				long long int meshStart = (index / squareProc) * (numRows * disp_width) + (index % squareProc) * numRows;
				
				//set result array to zero
					for(index = 0; index < numRows; index++){
						for(jndex = 0; jndex < numRows; jndex++){
							subR[((index) * numRows) + (jndex)] = 0;
						}
					}
				
				//iterate squareProc times
				for( iter = 0; iter < squareProc; iter++ ){
					
					//Multiply Matrices (storing results in subR)
						for(index = 0; index < numRows; index++){
							for(jndex = 0; jndex < numRows; jndex++){
								for(kndex = 0; kndex < numRows; kndex++){
									subR[((index) * numRows) + (jndex)] += 
										subA[index * numRows + kndex] * subB[jndex * numRows + kndex];
								}
							}
						}

					//Perform shifts
						//perform horizontal shifts on A and B 
							for(index = 0; index < squareProc; index++){
									Shift(subA, index, taskid, squareProc, numRows, false);
									Shift(subB, index, taskid, squareProc, numRows, true);
							}
					
				}
				
			}							

