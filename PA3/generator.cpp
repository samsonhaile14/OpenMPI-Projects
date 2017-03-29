
#include <stdio.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

#define MAX_SIZE	1000

main(){
	int i;
	int number_of_points;
	long random_num;
	ofstream fout;

	scanf("%d",&number_of_points);

	fout.open("set1.dat", ofstream::out);

	for(i=0;i<number_of_points;i++){
		random_num = random();
		fout << (random_num % MAX_SIZE) << '\n';
	}	
}
