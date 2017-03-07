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
#define RECEIVER 1

typedef struct complex{

	float real;
	float imag;

}complex;

//function specifications

/*outputs line of pixels to file*/
int pim_write_black_and_white_line(const char * const fileName,
                               const int width,
                               const int height,
                               unsigned char * pixels);

/*outputs pixel image to file*/
int pim_write_black_and_white(const char * const fileName,
                               const int width,
                               const int height,
                               unsigned char ** pixels);

/*calculates mandelbrot set value of complex number*/
int cal_pixel(complex c);

int main(int argc, char *argv[])
{

	//variables
	int x,y,disp_width,disp_height;
	double real_min, real_max, imag_min, imag_max;
	double scale_real, scale_imag;
	unsigned char **image;

	//initialization
	disp_width = 480;
	disp_height = 640;
	real_min = -2;
	real_max = 2;
	imag_min = -2;
	imag_max = 2;
	scale_real = (real_max - real_min)/((double)disp_width);
	scale_imag = (imag_max - imag_min)/((double)disp_height);

	image = malloc( sizeof(unsigned char *) * disp_width );

	for( x = 0; x < disp_width; x++ ){
		image[x] = malloc( sizeof( unsigned char) * disp_height );
	}

	//start timer
	double start = MPI_Wtime();

	//compute image
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
	printf("%f\n", end - start);

	//write image to file
	pim_write_black_and_white("mandelbrotImg", disp_height, 
				  disp_width,image);

	//free memory
	for( x = 0; x < disp_width; x++ ){
		free(image[x]);
	}

	free(image);


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

