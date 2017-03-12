//Sequential mandelbrot program
// by Samson Haile

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

#ifdef _WIN32
  #define WRITE_FLAGS "wb"
#else
  #define WRITE_FLAGS "w"
#endif

#define MASTER  0

typedef struct complex{

	float real;
	float imag;

}complex;

//function specifications

	/*outputs line of pixels to file
	  note: code was supplied by instructor and modified */
	int pim_write_black_and_white_line(const char * const fileName,
		                       const int width,
		                       const int height,
		                       unsigned char * pixels);

	/*outputs pixel image to file
	  note: code was supplied by instructor and modified */
	int pim_write_black_and_white(const char * const fileName,
		                       const int width,
		                       const int height,
		                       unsigned char ** pixels);

	/*calculates mandelbrot set value of complex number
	  note: code for function was taken from parallel textbook */
	int cal_pixel(complex c);

//main program
int main(int argc, char *argv[])
{

	//MPI variables and initalization
	int numtasks, taskid,len;
	int msgtag = 10;
	char hostname[MPI_MAX_PROCESSOR_NAME];

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD,&numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	MPI_Get_processor_name(hostname, &len);

	//Master and slave process variables
	int x,y, max_width, max_height,disp_width,disp_height;
	double real_min, real_max, imag_min, imag_max;
	double scale_real, scale_imag;

	max_width = 10000;
	max_height = 10000;
	disp_width = 500;
	disp_height = 500;
	real_min = -2;
	real_max = 2;
	imag_min = -2;
	imag_max = 2;

	//Master's work
	if( taskid == MASTER){
		//variables and allocation
		int *taskComplete;
		int **range;
		unsigned char **image;

		image = malloc( sizeof(unsigned char *) * max_width );
		taskComplete = malloc( sizeof(int) * numtasks );
		range = malloc( sizeof(int *) * numtasks );

		for( x = 0; x < numtasks; x++ ){
			taskComplete[x] = 0;
			range = malloc( sizeof( int ) * 2);
		}

		for( x = 0; x < max_width; x++ ){
			image[x] = malloc( sizeof( unsigned char) * max_height );
		}

		//distribute and receive work
		for(disp_width = 500; disp_width <= 500; disp_width ++){
			disp_height = disp_width;

			//start timer
			double start = MPI_Wtime();

			//assign work to processes
			int maxPos = (disp_width * disp_width)-1;

			range[0][0] = range[0][1] = -1;
			for( x = 1; x < numtasks; x++ ){
				range[x][0] = range[x-1][1] + 1;
				range[x][1] = range[x][0] + ((maxPos) / (numtasks - 1)) + 1;
				if( range[x][1] > maxPos ){
					range[x][1] = maxPos;
				}
				MPI_Send(range[x], 2, MPI_INT, x, msgtag,MPI_COMM_WORLD);
			}

			//wait for work back from processes
			int finished = 1;
			int* buffer;

			buffer = malloc( sizeof(int) * ((maxPos) / (numtasks - 1)) + 2);

			do{

				//assume all processes are finished until shown otherwise
				finished = 1;

				//check for work from other processes
				for( x = 1; x < numtasks; x++ ){
					int curComplete = 0;

					//check if process has completed task
					if(!taskComplete[x]){
						MPI_Iprobe( x, msgtag, MPI_COMM_WORLD, &curComplete, &status );
					}

					//if so, collect data from process
					if(curComplete){
						MPI_Receive( buffer, (range[x][1] - range[x][0] + 1), 
									 MPI_INT, x, msgtag, MPI_COMM_WORLD, &status);
						for( y = range[x][0]; y <= range[x][1]; y++ ){
							image[y / disp_width][y %disp_width] = buffer[ y - range[x][0]];
						}
						
						taskComplete[x] = 1;
					}

					/*if a task hasn't been completed now or in the 
					past, check all processes again on next iteration */
					else if(!taskComplete[x]){
						finished = 0;
					}
				}

			}while(!finished);

			//end timer
			double end = MPI_Wtime();

			//calculate elapsed time and output
			printf("%d, %f\n", disp_width, end - start);

			//write image to file (uncomment following two lines if image is desired)
			//smallest sized image (500x500) will be displayed
			if(disp_width == 500){
			pim_write_black_and_white("mandelbrotImg", disp_height, 
						  disp_width,image);
			}

			//deallocate for iteration
			free(buffer);

		}

		//free memory and terminate
		for( x = 0; x < disp_width; x++ ){
			free(image[x]);
		}

		free(image);
		free(taskComplete);
	}

	if(taskid != MASTER){
		//variables and allocation
		int range[2];
		int maxPos = (max_width * max_width)-1;
		int *buffer;

		buffer = malloc( sizeof(int) * ((maxPos) / (numtasks - 1)) + 2);

		//receive coordinate range and compute image
		//note: code was taken from textbook and modified
		for(disp_width = 500; disp_width <= 500; disp_width ++){
			disp_height = disp_width;

			scale_real = (real_max - real_min)/((double)disp_width);
			scale_imag = (imag_max - imag_min)/((double)disp_height);

			//Receive work range
			MPI_Recv( range, 2, MPI_INT, 0, msgtag,MPI_COMM_WORLD,&status )

			//Calculate pixels
			for( x = range[0]; x <= range[1]; x++ ){
				complex c;
				c.real = real_min + ((double) x * scale_real );
				c.imag = imag_min + ((double) y * scale_imag );				
				buffer[x - range[0]] = cal_pixel(c);
			}

			//Send results back to master
			MPI_Send( buffer, range[1]-range[0] + 1, MPI_INT, 0, 
					  msgtag, MPI_COMM_WORLD,&status);
		}
		free (buffer);
	}

	//terminate process
	MPI_Finalize();


}

int pim_write_black_and_white_line(const char * const fileName,
                               const int width,
                               const int height,
                               unsigned char * pixels)
{
  FILE * fp = fopen(fileName, WRITE_FLAGS);

  if (!fp) return 0;
  fprintf(fp, "P5\n%i %i 255\n", width, height);
  fwrite(pixels, width * height, 1, fp);
  fclose(fp);

  return 1;
}

int pim_write_black_and_white(const char * const fileName,
                               const int width,
                               const int height,
                               unsigned char ** pixels)
{
  int i;
  int ret;
  unsigned char * t = malloc(sizeof(unsigned char) * width * height);

  for (i = 0; i < height; ++i) memcpy(t + width * i, pixels[i], width);
  ret = pim_write_black_and_white_line(fileName, width, height, t);
  free(t);
  return ret;
}

int cal_pixel(complex c){

	int count, max_iter;
	complex z;
	float temp, lengthsq;

	max_iter = 256;
	z.real = 0;
	z.imag = 0;
	count = 0;
	do{

		temp = z.real * z.real - z.imag * z.imag + c.real;
		z.imag = 2 * z.real * z.imag + c.imag;
		z.real = temp;
		lengthsq = z.real * z.real + z.imag + z.imag;
		count++;
	}while((lengthsq < 4.0) && (count < max_iter));
	
}

