//Parallel bucket sort program
// by Samson Haile

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "mpi.h"

#define MASTER 0

using namespace std;

//function declarations
void insertionSort( vector<int> &dSet, long long int size );

//main program
int main(int argc, char *argv[])
{
  
	//common variables
		long long int act_size,temp;
		long long int index;
		long long int max_size;
		vector< vector<int> > buckets;

		int taskid,numtasks;
		int msgtag = 10;
		MPI_Status status;

	//initialization
		MPI_Init(&argc, &argv);
		MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
		MPI_Comm_rank(MPI_COMM_WORLD,&taskid);

		//read largest size to be used
		if( argc < 1){
			return 1;
		}
		
		max_size = atoll(argv[1]);
		
	//Set appropriate number of buckets
		for(index = 0; index < numtasks; index++){
			vector<int> vTemp(max_size);
			buckets.push_back(vTemp);
		}

	if( taskid == MASTER ){   
		//generate constant data
		vector<int> dSet(max_size);
		vector<int> sBucket(max_size);

		//data read
		for(index = 0; index < max_size; index++){
			dSet[index] = (random() % 10000);
		}
				
		//Sort across different sizes
		for(act_size = max_size/5; act_size <= max_size; act_size += max_size/5){
			vector< long long int > bSize(numtasks, 0);
			
			int len = act_size / numtasks;
			int pos = 0;
			int max = -1;

			//find the largest data point
			for(index = 0; index < act_size;index++){
				if(max < dSet[index]){
					max = dSet[index];
				}
			}

			//distribute data to all other processes
			for( index = 1; index < numtasks; index++ ){

				//first send size of variable sized array
					MPI_Send( &len, 1, MPI_INT, index, msgtag, MPI_COMM_WORLD );

				//receive variable sized array( msgtag changed to preserve send order )
				if(len != 0){
					MPI_Send( &dSet[pos], len, MPI_INT, index, msgtag + 1, MPI_COMM_WORLD );
				}

				//send max val
					MPI_Send( &max, 1, MPI_INT, index, msgtag + 2, MPI_COMM_WORLD );

				//change start position of array
					pos += len;
			}
			
			//designate last set of numbers as bucket for master
				copy( dSet.begin() + pos, dSet.begin() + act_size, sBucket.begin() );

				pos = act_size - pos;
				
			//ensure all processes start work at same time
				MPI_Barrier(MPI_COMM_WORLD);
				
			//start timer
				double start = MPI_Wtime();
				
			//organize data into respective buckets
			for(index = 0; index < pos;index++){
			  temp = dSet[index] / (max / numtasks);
			  if( temp >= numtasks ){
				temp = numtasks - 1;
			  }
			  buckets[ temp ][bSize[temp]] = (data[index]);
			  bSize[temp]++;
			}

			//send each bucket to appropriate task
				//receive bucket from self
					copy( buckets[taskid].begin(), buckets[taskid].begin() + bSize[taskid],
							sBucket.begin());

					pos = bSize[taskid];

				//Perform communication for circulating buckets
				for( index = 0; index < numtasks; index++ ){
					
					//turn to send if index == taskid
					if(index == taskid){
						for( temp = 0; temp < numtasks; temp++ ){
							if(temp != taskid){
								//get size of bucket
									len = bSize[temp];

								//first send size of variable sized array
									MPI_Send( &len, 1, MPI_INT, temp, msgtag, MPI_COMM_WORLD );

								//receive variable sized array( msgtag changed to preserve send order )
								if( len != 0 ){
									MPI_Send( &(buckets[temp][0]), len, MPI_INT, temp, msgtag + 1, MPI_COMM_WORLD );
								}
							}
						}
					}

					//else, task is on receiving end
					else{
						//first send size of variable sized array
							MPI_Recv( &len, 1, MPI_INT, index, msgtag, MPI_COMM_WORLD, &status );

						//receive variable sized array( msgtag changed to preserve send order )
						if(len != 0){
							MPI_Recv( &sBucket[pos], len, MPI_INT, index, msgtag + 1, MPI_COMM_WORLD, &status );
						}

						//change start position of array
							pos += len;						
					}

					MPI_Barrier(MPI_COMM_WORLD); //wait till all tasks finish receiving data

				}

				
			//sort own bucket
				insertionSort( sBucket, pos );

			MPI_Barrier(MPI_COMM_WORLD);	//block to get accurate timing

			//end timer
				double end = MPI_Wtime();

			//calculate elapsed time and output
		  		printf("%d, %lld, %f\n", numtasks, act_size, end - start);

			//clear buckets
				for(index = 0; index < numtasks; index++){
					buckets[index].clear();
				}

		}
	}

	else{
		vector<int> sBucket(max_size);
		//Sort across different sizes
		for(act_size = max_size / 5; act_size <= max_size; act_size += max_size/5){
			vector<long long int> bSize;
			int pos = 0;
			int max = -1;
			int len;

			//Receive data from master

				//first send size of variable sized array
					MPI_Recv( &len, 1, MPI_INT, 0, msgtag, MPI_COMM_WORLD, &status );

				//receive variable sized array( msgtag changed to preserve send order )
				if(len != 0){
					MPI_Recv( &sBucket[0], len, MPI_INT, 0, msgtag + 1, MPI_COMM_WORLD, &status );
				}

				//receive max val
					MPI_Recv( &max, 1, MPI_INT, 0, msgtag + 2, MPI_COMM_WORLD, &status );

			//ensure all processes start work at same time
				MPI_Barrier(MPI_COMM_WORLD);

			//organize data into respective buckets
				for(index = 0; index < len; index++){
				  temp = dSet[index] / (max / numtasks);
				  if( temp >= numtasks ){
					temp = numtasks - 1;
				  }
				  buckets[ temp ][bSize[temp]] = (data[index]);
				  bSize[temp]++;
				}

			//receive bucket from self
				copy( buckets[taskid].begin(), buckets[taskid].begin() + bSize[taskid], sBucket.begin());

				pos = bSize[taskid];

			//Perform communication for circulating buckets
				for( index = 0; index < numtasks; index++ ){
					
					//turn to send if index == taskid
					if(index == taskid){
						for( temp = 0; temp < numtasks; temp++ ){
							if(temp != taskid){
								//get size of bucket
									len = bSize[temp];

								//first send size of variable sized array
									MPI_Send( &len, 1, MPI_INT, temp, msgtag, MPI_COMM_WORLD );

								//receive variable sized array( msgtag changed to preserve send order )
								if( len != 0 ){
									MPI_Send( &(buckets[temp][0]), len, MPI_INT, temp, msgtag + 1, MPI_COMM_WORLD );
								}
							}
						}
					}

					//else, task is on receiving end
					else{
						//first send size of variable sized array
							MPI_Recv( &len, 1, MPI_INT, index, msgtag, MPI_COMM_WORLD, &status );

						//receive variable sized array( msgtag changed to preserve send order )
						if(len != 0){
							MPI_Recv( &sBucket[pos], len, MPI_INT, index, msgtag + 1, MPI_COMM_WORLD, &status );
						}

						//change start position of array
							pos += len;						
					}

					MPI_Barrier(MPI_COMM_WORLD); //wait till all tasks finish receiving data

				}

			//sort own bucket
			  insertionSort( sBucket, pos );

			MPI_Barrier(MPI_COMM_WORLD);	//block to get accurate timing

			//clear buckets
				for(index = 0; index < numtasks; index++){
					buckets[index].clear();
				}

		}

	}

	//terminate
		MPI_Finalize();

}

void insertionSort( vector<int> &dSet, long long int size ){

	long long int curs = 0,index;
	int temp;

	//perform insertion sort for each value in array
	for( curs = 0; curs < size; curs++ ){
		//save value for insertion
			temp = dSet[curs];

		//move value down array
		for(index = curs; (index > 0) && (dSet[index - 1] > temp); index--){
		  dSet[index] = dSet[index - 1];
		}

		//reinsert value
			dSet[index] = temp;

	}

}


