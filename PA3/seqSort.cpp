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
void insertionSort( vector<int> &dSet, long long int size );

//main program
int main(int argc, char *argv[])
{
  
	//variables
		long long int act_size,temp;
		long long int index;
		long long int max_size;
	    vector< vector<int> > buckets;
		int bucketCount;

		//initialization
		MPI_Init(&argc, &argv);	//only used for timer
	       
		if( argc < 2){
			return 1;
		}

		bucketCount = atoi( argv[1] );		
		max_size = atoll(argv[2]);

		
		//allocating enough buckets
		for( index = 0; index < bucketCount; index++ ){
			vector<long long int> bTemp(max_size);

			buckets.push_back(bTemp);
		}

	//generate constant data
		vector<int> data(max_size);

		//data read
		for(index = 0; index < max_size; index++){
			data[index] = (random() % 10000);
		}
 
	//Sort across different sizes
	for(act_size = max_size; act_size <= data.size(); act_size += max_size/5){
		int max = data[0];
		vector<long long int> bSize(bucketCount, 0);
		
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
		  buckets[ temp ][bSize[temp]] = (data[index]);
		  bSize[temp]++;
		}

		//sort each bucket
		for(index = 0; index < bucketCount; index++){
		  insertionSort( buckets[index], bSize[index] );
		}

		//end timer
			double end = MPI_Wtime();

		//calculate elapsed time and output
      			printf("%d, %lld, %f\n", bucketCount, act_size, end - start);

	}

	//terminate
		MPI_Finalize();

}

//switched for standard sort
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

