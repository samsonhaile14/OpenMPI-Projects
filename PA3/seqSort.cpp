//Sequential bucket sort program
// by Samson Haile

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <fstream>
#include "mpi.h"

using namespace std;

//function specifications
void insertionSort( vector<int> &dSet );

//main program
int main(int argc, char *argv[])
{
  
	//variables
		long long int act_size,temp;
		long long int index;
		vector<int> data;
	    vector< vector<int> > buckets;
		int bucketCount;

		//initialization
		MPI_Init(&argc, &argv);	//only used for timer
	       
		if( argc < 1){
			return 1;
		}

		bucketCount = atoi( argv[1] );

		//allocating enough buckets
		for( index = 0; index < bucketCount; index++ ){
			vector<int> bTemp;

			buckets.push_back(bTemp);
		}

	//generate constant data

		//data read
		for(index = 0; index < 2500000000; index++){
			data.push_back(random() % 10000);
		}
 	
	//Sort across different sizes
	for(act_size = 500000000; act_size <= data.size(); act_size += 500000000){
		int max = -1;

		//start timer
		double start = MPI_Wtime();

		//find the largest data point
		for(index = 0; index < act_size;index++){
			if(max < data[index]){
				max = data[index];
			}
		}

		//organize data into respective buckets
		for(index = 0; index < act_size;index++){
		  temp = data[index] / (max / bucketCount);
		  if( temp >= bucketCount ){
		    temp = bucketCount - 1;
		  }
		  buckets[ temp ].push_back(data[index]);
		}

		//sort each bucket
		for(index = 0; index < bucketCount; index++){
		  sort( buckets[index].begin(), buckets[index].end() );
		}

		//end timer
			double end = MPI_Wtime();

		//calculate elapsed time and output
      			printf("%d, %lld, %f\n", bucketCount, act_size, end - start);

		//Clear buckets
			for(index = 0; index < bucketCount; index++ ){
			  buckets[index].clear();
			}
	}

	//terminate
		MPI_Finalize();

}

//switched for standard sort
void insertionSort( vector<int> &dSet ){

	int curs = 0,index;
	int temp;

	//perform insertion sort for each value in array
	for( curs = 0; curs < dSet.size(); curs++ ){
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

