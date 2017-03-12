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

	//variables
	int x,y, max_width, max_height,disp_width,disp_height;
	double real_min, real_max, imag_min, imag_max;
	double scale_real, scale_imag;
	unsigned char **image;

	//initialization
	MPI_Init(&argc, &argv);	//only used for timer

	max_width = max_height = 1000;
	real_min = -2;
	real_max = 2;
	imag_min = -2;
	imag_max = 2;

	image = malloc( sizeof(unsigned char *) * max_width );

	for( x = 0; x < max_width; x++ ){
		image[x] = malloc( sizeof( unsigned char) * max_height );
	}

	//compute image
	//note: code was taken from textbook and modified
	for(disp_width = 500; disp_width <= max_width; disp_width ++){
		disp_height = disp_width;

		//start timer
		double start = MPI_Wtime();

		scale_real = (real_max - real_min)/((double)disp_width);
		scale_imag = (imag_max - imag_min)/((double)disp_height);

		for(x = 0; x < disp_width; x++){
			for( y = 0; y < disp_height; y++ ){
				complex c;
				c.real = real_min + ((double) x * scale_real );
				c.imag = imag_min + ((double) y * scale_imag );
				image[x][y] = cal_pixel(c);//calculate pixel
							   //and store in buffer
			}
		}

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
	}

	//free memory and terminate
	for( x = 0; x < disp_width; x++ ){
		free(image[x]);
	}

	free(image);

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

	return count;
	
}

