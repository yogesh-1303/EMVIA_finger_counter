

/*The program follows the parallel thinning algorithm for skeletonization as described in Davies table/equation 9.13 and 9.14. Kindly refer to the part for detailed understanding*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define WIDTH 		320
#define HEIGHT 		240
#define PIX_MAX 	255
#define NUM_IMGS	100

typedef struct{
	char filenames[NUM_IMGS+1][30];
	FILE	*img[NUM_IMGS+1];
}file;

typedef struct{
	 uint8_t red[WIDTH][HEIGHT];
	 uint8_t green[WIDTH][HEIGHT];
	 uint8_t blue[WIDTH][HEIGHT];  //[NUM_IMGS+1]
}colors;

using namespace std;

char header[NUM_IMGS][100];
uint8_t arr[NUM_IMGS+1][320][240];
//uint8_t arr2[320][240];
int x,y,i;

//background elimination through frame differencing
//this operation is followed by thresholding for appropriate results
void backgrnd_el(){
	if(i>1)				//there is no Zeroth image and hence i starts with 2
	arr[i][y][x] = arr[i][y][x] - arr[i-1][y][x];
}

//function to threshold the image
//takes threshold value as argument and returns void
void threshold(int val){
	
			if(arr[i][y][x]<val) 
				arr[i][y][x] = 0;
			else	
				arr[i][y][x] = 255;
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
    sigma = arr[i][y][x+1] + arr[i][y-1][x+1] + arr[i][y-1][x] + 
							arr[i][y-1][x-1] + arr[i][y][x-1]
							+ arr[i][y+1][x-1] + arr[i][y+1][x] + arr[i][y+1][x+1];
	
	//calculating the value of chi (comes out as 2, 4, 6, or 8)		
	chi = (arr[i][y][x+1] != arr[i][y-1][x])+(arr[i][y-1][x] != arr[i][y][x-1])
		+ (arr[i][y][x-1] != arr[i][y+1][x]) + (arr[i][y+1][x] != arr[i][y][x+1])
		+2 *((arr[i][y-1][x+1] > arr[i][y][x+1])&&(arr[i][y-1][x+1] > arr[i][y-1][x])
		+ (arr[i][y-1][x-1] > arr[i][y-1][x])&&(arr[i][y-1][x-1] > arr[i][y][x-1])
		+ (arr[i][y+1][x-1] > arr[i][y][x-1])&&(arr[i][y+1][x-1] > arr[i][y+1][x])
		+ (arr[i][y+1][x+1] > arr[i][y+1][x])&&(arr[i][y+1][x+1] > arr[i][y][x+1]));
	
	//stripping off the north point	
	if((arr[i][y-1][x]==0)&&(arr[i][y][x]==255)&&(arr[i][y+1][x]==255)&& //North Point
			(chi == 2)&&(sigma != 255)){
				arr[i][y][x] = 0;
				finished = false;
				}
	
	 }
	}
	//raster through the image for stripping off the south points
		for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 	//sigma = A1 + A2 + A3 + A4 + A5 + A6 + A7 + A8
    sigma = arr[i][y][x+1] + arr[i][y-1][x+1] + arr[i][y-1][x] + 
							arr[i][y-1][x-1] + arr[i][y][x-1]
							+ arr[i][y+1][x-1] + arr[i][y+1][x] + arr[i][y+1][x+1];
	
	//calculating the value of chi (comes out as 2, 4, 6, or 8)			
	chi = (arr[i][y][x+1] != arr[i][y-1][x])+(arr[i][y-1][x] != arr[i][y][x-1])
		+ (arr[i][y][x-1] != arr[i][y+1][x]) + (arr[i][y+1][x] != arr[i][y][x+1])
		+2 *((arr[i][y-1][x+1] > arr[i][y][x+1])&&(arr[i][y-1][x+1] > arr[i][y-1][x])
		+ (arr[i][y-1][x-1] > arr[i][y-1][x])&&(arr[i][y-1][x-1] > arr[i][y][x-1])
		+ (arr[i][y+1][x-1] > arr[i][y][x-1])&&(arr[i][y+1][x-1] > arr[i][y+1][x])
		+ (arr[i][y+1][x+1] > arr[i][y+1][x])&&(arr[i][y+1][x+1] > arr[i][y][x+1]));
	
	//stripping off the south point	
	if((arr[i][y-1][x]==255)&&(arr[i][y][x]==255)&&(arr[i][y+1][x]==0)&&//south point
			(chi == 2)&&(sigma != 255)){
				arr[i][y][x] = 0;
				finished = false;
				}
	
	 }
	}
	//raster through the image for stripping off the east points
		for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
    sigma = arr[i][y][x+1] + arr[i][y-1][x+1] + arr[i][y-1][x] + 
							arr[i][y-1][x-1] + arr[i][y][x-1]
							+ arr[i][y+1][x-1] + arr[i][y+1][x] + arr[i][y+1][x+1];
			
	chi = (arr[i][y][x+1] != arr[i][y-1][x])+(arr[i][y-1][x] != arr[i][y][x-1])
		+ (arr[i][y][x-1] != arr[i][y+1][x]) + (arr[i][y+1][x] != arr[i][y][x+1])
		+2 *((arr[i][y-1][x+1] > arr[i][y][x+1])&&(arr[i][y-1][x+1] > arr[i][y-1][x])
		+ (arr[i][y-1][x-1] > arr[i][y-1][x])&&(arr[i][y-1][x-1] > arr[i][y][x-1])
		+ (arr[i][y+1][x-1] > arr[i][y][x-1])&&(arr[i][y+1][x-1] > arr[i][y+1][x])
		+ (arr[i][y+1][x+1] > arr[i][y+1][x])&&(arr[i][y+1][x+1] > arr[i][y][x+1]));
		
	if((arr[i][y-1][x]==0)&&(arr[i][y][x]==255)&&(arr[i][y][x-1]==255)&&//east point
			(chi == 2)&&(sigma != 255)){
				arr[i][y][x] = 0;
				finished = false;
				}
	
	 }
	}
	
		for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
    sigma = arr[i][y][x+1] + arr[i][y-1][x+1] + arr[i][y-1][x] + 
							arr[i][y-1][x-1] + arr[i][y][x-1]
							+ arr[i][y+1][x-1] + arr[i][y+1][x] + arr[i][y+1][x+1];
			
	chi = (arr[i][y][x+1] != arr[i][y-1][x])+(arr[i][y-1][x] != arr[i][y][x-1])
		+ (arr[i][y][x-1] != arr[i][y+1][x]) + (arr[i][y+1][x] != arr[i][y][x+1])
		+2 *((arr[i][y-1][x+1] > arr[i][y][x+1])&&(arr[i][y-1][x+1] > arr[i][y-1][x])
		+ (arr[i][y-1][x-1] > arr[i][y-1][x])&&(arr[i][y-1][x-1] > arr[i][y][x-1])
		+ (arr[i][y+1][x-1] > arr[i][y][x-1])&&(arr[i][y+1][x-1] > arr[i][y+1][x])
		+ (arr[i][y+1][x+1] > arr[i][y+1][x])&&(arr[i][y+1][x+1] > arr[i][y][x+1]));
		
	if((arr[i][y-1][x]==255)&&(arr[i][y][x]==255)&&(arr[i][y][x-1]==0)&& //west point
			(chi == 2)&&(sigma != 255)){
				arr[i][y][x] = 0;
				finished = false;
				}
	 }
	}
 }while(finished==0);
	
}



//main function
int main(int argc, char *argv[]){


	colors color;
	
	file input, output;
	for(i=1;i<=NUM_IMGS;i++){
	//input and output files
	sprintf(input.filenames[i], "Arm_img%d.ppm", i);
	sprintf(output.filenames[i], "New_arm%d.pgm", i);
	//checking for error in opening files
	input.img[i] = fopen(input.filenames[i], "rb");
		if(input.img[i] == NULL)
		{
			printf("\nError opening image");
			exit(-1);
		}
		
		output.img[i] = fopen(output.filenames[i], "wb+");
		
		if(output.img[i] == NULL)
		{
			printf("\nError opening image");
			exit(-1);
		}
	
	//scanning the header of input image
	for(int j=0; j<17; j++){
	 fscanf(input.img[i], "%c", header[i]+j);
	  //printf("%x \n", header[j]);
	 }
	 //puts(header);
	 header[i][17] = '\0';
	 
	 puts(input.filenames[i]);
	puts(output.filenames[i]);
	 
	 //writing the header to the output file
	 for(int j=0; j<17; j++){
	 if(j==1) 
	 	fprintf(output.img[i], "%c", '5');
	 else
		fprintf(output.img[i], "%c", header[i][j]);
	 //printf("\t%x",header[i][j]);
	 }
	 //puts(header[i]);
	 header[i][17] = '\0';
	 
	 //raster through the image for scanning and converting it into a 
	 //binary image
	 for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 	
	 	//scan the pixels (1 byte each for RGB)
	 		fscanf(input.img[i], "%c", color.red[y]+x);
			fscanf(input.img[i], "%c", color.green[y]+x);
			fscanf(input.img[i], "%c", color.blue[y]+x);
		
		//convert into gray scale using the formula 
		//G = 0.299R + 0.587G + 0.114B
		
			arr[i][y][x] =((float)color.red[y][x] * 0.299) + 
						((float)color.green[y][x] * 0.587) + 
						((float)color.blue[y][x] * 0.114);
					
			backgrnd_el();			
		//convert into binary image by thresholding	
			threshold(100);	
			
			//fprintf(output.img[i], "%c", arr[i][y][x]);
			//printf("%d", arr[y][x]);
				
	 }
	}
	thinning_algo();

	//writing the final pixel values to the output image
	for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 		fprintf(output.img[i], "%c", arr[i][y][x]);
	 	}
	 }
	}
	return 0;
}
