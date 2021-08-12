

/*The program follows the parallel thinning algorithm for skeletonization as described in Davies table/equation 9.13 and 9.14. Kindly refer to the part for detailed understanding*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define WIDTH 		320
#define HEIGHT 		240
#define PIX_MAX 	255

typedef struct{
	uint8_t red[WIDTH][HEIGHT];
	uint8_t green[WIDTH][HEIGHT];
	uint8_t blue[WIDTH][HEIGHT];	
}colors;

using namespace std;

char header[100];
uint8_t arr[320][240];
//uint8_t arr2[320][240];
int x,y;

//function to threshold the image
//takes threshold value as argument and returns void
void threshold(int val){
	
			if(arr[y][x]<val) 
				arr[y][x] = 0;
			else	
				arr[y][x] = 255;
}

//parallel algorithm for skeletonization
//takes and returns void
//strips off north, south, east, and west pixels until thinning is completed
//basically rasters through the image cyclically for stripping off 
//north, south, east, and west points. Four similar looping happening 
//in the function untill no change in pixels is noticed 
void thinning_algo(){

	bool finished;
	int sigma,chi;
	do{
	finished = true;
	
	//raster through the image for stripping off the north points
	for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 	
	//sigma = A1 + A2 + A3 + A4 + A5 + A6 + A7 + A8
    sigma = arr[y][x+1] + arr[y-1][x+1] + arr[y-1][x] + 
							arr[y-1][x-1] + arr[y][x-1]
							+ arr[y+1][x-1] + arr[y+1][x] + arr[y+1][x+1];
	
	//calculating the value of chi (comes out as 2, 4, 6, or 8)		
	chi = (arr[y][x+1] != arr[y-1][x])+(arr[y-1][x] != arr[y][x-1])
		+ (arr[y][x-1] != arr[y+1][x]) + (arr[y+1][x] != arr[y][x+1])
		+2 *((arr[y-1][x+1] > arr[y][x+1])&&(arr[y-1][x+1] > arr[y-1][x])
		+ (arr[y-1][x-1] > arr[y-1][x])&&(arr[y-1][x-1] > arr[y][x-1])
		+ (arr[y+1][x-1] > arr[y][x-1])&&(arr[y+1][x-1] > arr[y+1][x])
		+ (arr[y+1][x+1] > arr[y+1][x])&&(arr[y+1][x+1] > arr[y][x+1]));
	
	//stripping off the north point	
	if((arr[y-1][x]==0)&&(arr[y][x]==255)&&(arr[y+1][x]==255)&& //North Point
			(chi == 2)&&(sigma != 255)){
				arr[y][x] = 0;
				finished = false;
				}
	
	 }
	}
	//raster through the image for stripping off the south points
		for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 	//sigma = A1 + A2 + A3 + A4 + A5 + A6 + A7 + A8
    sigma = arr[y][x+1] + arr[y-1][x+1] + arr[y-1][x] + 
							arr[y-1][x-1] + arr[y][x-1]
							+ arr[y+1][x-1] + arr[y+1][x] + arr[y+1][x+1];
	
	//calculating the value of chi (comes out as 2, 4, 6, or 8)			
	chi = (arr[y][x+1] != arr[y-1][x])+(arr[y-1][x] != arr[y][x-1])
		+ (arr[y][x-1] != arr[y+1][x]) + (arr[y+1][x] != arr[y][x+1])
		+2 *((arr[y-1][x+1] > arr[y][x+1])&&(arr[y-1][x+1] > arr[y-1][x])
		+ (arr[y-1][x-1] > arr[y-1][x])&&(arr[y-1][x-1] > arr[y][x-1])
		+ (arr[y+1][x-1] > arr[y][x-1])&&(arr[y+1][x-1] > arr[y+1][x])
		+ (arr[y+1][x+1] > arr[y+1][x])&&(arr[y+1][x+1] > arr[y][x+1]));
	
	//stripping off the south point	
	if((arr[y-1][x]==255)&&(arr[y][x]==255)&&(arr[y+1][x]==0)&&//south point
			(chi == 2)&&(sigma != 255)){
				arr[y][x] = 0;
				finished = false;
				}
	
	 }
	}
	//raster through the image for stripping off the east points
		for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
    sigma = arr[y][x+1] + arr[y-1][x+1] + arr[y-1][x] + 
							arr[y-1][x-1] + arr[y][x-1]
							+ arr[y+1][x-1] + arr[y+1][x] + arr[y+1][x+1];
			
	chi = (arr[y][x+1] != arr[y-1][x])+(arr[y-1][x] != arr[y][x-1])
		+ (arr[y][x-1] != arr[y+1][x]) + (arr[y+1][x] != arr[y][x+1])
		+2 *((arr[y-1][x+1] > arr[y][x+1])&&(arr[y-1][x+1] > arr[y-1][x])
		+ (arr[y-1][x-1] > arr[y-1][x])&&(arr[y-1][x-1] > arr[y][x-1])
		+ (arr[y+1][x-1] > arr[y][x-1])&&(arr[y+1][x-1] > arr[y+1][x])
		+ (arr[y+1][x+1] > arr[y+1][x])&&(arr[y+1][x+1] > arr[y][x+1]));
		
	if((arr[y-1][x]==0)&&(arr[y][x]==255)&&(arr[y][x-1]==255)&&//east point
			(chi == 2)&&(sigma != 255)){
				arr[y][x] = 0;
				finished = false;
				}
	
	 }
	}
	
		for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
    sigma = arr[y][x+1] + arr[y-1][x+1] + arr[y-1][x] + 
							arr[y-1][x-1] + arr[y][x-1]
							+ arr[y+1][x-1] + arr[y+1][x] + arr[y+1][x+1];
			
	chi = (arr[y][x+1] != arr[y-1][x])+(arr[y-1][x] != arr[y][x-1])
		+ (arr[y][x-1] != arr[y+1][x]) + (arr[y+1][x] != arr[y][x+1])
		+2 *((arr[y-1][x+1] > arr[y][x+1])&&(arr[y-1][x+1] > arr[y-1][x])
		+ (arr[y-1][x-1] > arr[y-1][x])&&(arr[y-1][x-1] > arr[y][x-1])
		+ (arr[y+1][x-1] > arr[y][x-1])&&(arr[y+1][x-1] > arr[y+1][x])
		+ (arr[y+1][x+1] > arr[y+1][x])&&(arr[y+1][x+1] > arr[y][x+1]));
		
	if((arr[y-1][x]==255)&&(arr[y][x]==255)&&(arr[y][x-1]==0)&& //west point
			(chi == 2)&&(sigma != 255)){
				arr[y][x] = 0;
				finished = false;
				}
	 }
	}
 }while(finished==0);
	
}


//main function
int main(int argc, char *argv[]){
	colors color;
	
	//input and output files
	FILE *in_img = fopen("Arm320x240.ppm", "rb");
	FILE *out_img = fopen("arm.pgm", "wb+");
	
	if(in_img == NULL)
	{
		printf("\nError opening image");
		exit(-1);
	}
	
	//scanning the header of input image
	for(int i=0; i<53; i++){
	 fscanf(in_img, "%c", header+i);
	  //printf("%x \n", header[i]);
	 }
	 //puts(header);
	 header[53] = '\0';
	 
	 //writing the header to the output file
	 for(int i=0; i<53; i++){
	 if(i==1) 
	 	fprintf(out_img, "%c", '5');
	 else
		fprintf(out_img, "%c", header[i]);
	 printf("\t%x",header[i]);
	 }
	 puts(header);
	 header[53] = '\0';
	 
	 //raster through the image for scanning and converting it into a 
	 //binary image
	 for( y=0; y<HEIGHT; y++){
	 		//printf("\n");
	 	for( x=0; x<WIDTH; x++){
	 	
	 	//scan the pixels (1 byte each for RGB)
	 		fscanf(in_img, "%c", color.red[y]+x);
			fscanf(in_img, "%c", color.green[y]+x);
			fscanf(in_img, "%c", color.blue[y]+x);
		
		//convert into gray scale using the formula 
		//G = 0.299R + 0.587G + 0.114B
			arr[y][x] = ((float)color.red[y][x] * 0.299) + 
						((float)color.green[y][x] * 0.587) + 
						((float)color.blue[y][x] * 0.114);
						
		//convert into binary image by thresholding	
			threshold(200);	
			
			//fprintf(out_img, "%c", arr[y][x]);
			//printf("%d", arr[y][x]);
				
	 }
	}
	thinning_algo();

	//writing the final pixel values to the output image
	for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 		fprintf(out_img, "%c", arr[y][x]);
	 	}
	 }
	return 0;
}
