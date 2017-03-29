//Sequential bucket sort program
// by Samson Haile

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include "mpi.h"

using namespace std;

//function specifications
void insertionSort( vector<int> dSet );

//main program
int main(int argc, char *argv[])
{

	//variables
		int act_size,temp;
		int index;
		vector<int> data;

		//initialization
		MPI_Init(&argc, &argv);	//only used for timer

	//Read data from file
		ifstream fin;

		//error checking
		if(argc > 1){
			fin.open( argv[1], ifstream::in )
		}
		else{
			return 1;
		}

		//data read
		while(fin.good()){
			fin >> temp;
			data.push_back(temp);
		}

		fin.close();

	//designate array for sorted results
		vector<int> result(data.size(),0 );

	//Sort across different sizes
	for(act_size = 500; act_size < data.size(); act_size += 500){
		vector<int> buckets[100];
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
			buckets[ data[index] / (max / 100) ].push_back(data[index]);
		}

		//sort each bucket
		for(index = 0; index < 100; index++){
			insertionSort( buckets[index] );
		}

		//place into result array
		int curs = 0, bIndex;
		for(index = 0; index < 100; index++){
			for(bIndex = 0; bIndex < buckets[index].size(); bIndex++,curs++){
				result[curs] = buckets[index][bIndex];
			}
		}

		//end timer
			double end = MPI_Wtime();

		//calculate elapsed time and output
			printf("%d, %f\n", disp_width, end - start);

	}

	//terminate
		MPI_Finalize();

}


void insertionSort( vector<int> dSet ){

	int curs = 0,index;
	int temp;

	//perform insertion sort for each value in array
	for( curs = 0; curs < dSet.size(); curs++ ){
		//save value for insertion
			temp = dSet[curs];

		//move value down array
		for(index = curs; (index > 0) && (dSet[index - 1] > dSet[index]); index--){
			dSet[index] = dSet[index - 1];
		}

		//reinsert value
			dSet[curs] = temp;

	}

}

