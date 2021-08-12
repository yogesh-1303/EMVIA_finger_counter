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
uint8_t arr2[320][240];
int x,y;

void threshold(int val){
	
			if(arr[y][x]<val) 
				arr[y][x] = 0;
			else	
				arr[y][x] = 255;
}


//#define PARALLEL_THIN

typedef enum{
 NORTH, 
 SOUTH,
 EAST,
 WEST
}strip;

bool finished;

void thinning_algo(/*strip dir*/){

	
	int sigma,chi;
	do{
	finished = true;
	for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
    sigma = arr[y][x+1] + arr[y-1][x+1] + arr[y-1][x] + 
							arr[y-1][x-1] + arr[y][x-1]
							+ arr[y+1][x-1] + arr[y+1][x] + arr[y+1][x+1];
			
	//int chi = 0;
	chi = (arr[y][x+1] != arr[y-1][x])+(arr[y-1][x] != arr[y][x-1])
		+ (arr[y][x-1] != arr[y+1][x]) + (arr[y+1][x] != arr[y][x+1])
		+2 *((arr[y-1][x+1] > arr[y][x+1])&&(arr[y-1][x+1] > arr[y-1][x])
		+ (arr[y-1][x-1] > arr[y-1][x])&&(arr[y-1][x-1] > arr[y][x-1])
		+ (arr[y+1][x-1] > arr[y][x-1])&&(arr[y+1][x-1] > arr[y+1][x])
		+ (arr[y+1][x+1] > arr[y+1][x])&&(arr[y+1][x+1] > arr[y][x+1]));
		
	if((arr[y][x]==255)&&(chi==2)&&(sigma != 255)){
		arr[y][x] = 0;
		finished = false;
		}
		}
	  }
	}while(finished==0); 
#ifdef PARALLEL_THIN
	
	switch(dir){
	//stripping north points
	case NORTH:
	if((arr[y-1][x]==0)&&(arr[y][x]==255)&&(arr[y+1][x]==255)&&
			(chi == 2)&&(sigma != 255)){
				arr2[y][x] = 0;
				finished = false;
				}
			else
				arr2[y][x] = arr[y][x];
			break;
	//stripping south points
	case SOUTH:		
	if((arr[y-1][x]==255)&&(arr[y][x]==255)&&(arr[y+1][x]==0)&&
			(chi == 2)&&(sigma != 255)){
				arr2[y][x] = 0;
				finished = false;
				}
			else
				arr2[y][x] = arr[y][x];
			break;
	//stripping east points
	case EAST:		
	if((arr[y-1][x]==0)&&(arr[y][x]==255)&&(arr[y][x-1]==255)&&
			(chi == 2)&&(sigma != 255)){
				arr2[y][x] = 0;
				finished = false;
				}
			else
				arr2[y][x] = arr[y][x];
			break;
	//stripping west points
	case WEST:		
	if((arr[y-1][x]==255)&&(arr[y][x]==255)&&(arr[y][x-1]==0)&&
			(chi == 2)&&(sigma != 255)){
				arr2[y][x] = 0;
				finished = false;
				}
			else
				arr2[y][x] = arr[y][x];
			break;
	}
	
#endif
}

int main(int argc, char *argv[]){
	colors color;
	FILE *in_img = fopen("Arm320x240.ppm", "rb");
	FILE *out_img = fopen("arm.pgm", "wb+");
	
	if(in_img == NULL)
	{
		printf("\nError opening image");
		exit(-1);
	}
	
	for(int i=0; i<53; i++){
	 fscanf(in_img, "%c", header+i);
	  printf("%x \n", header[i]);
	 }
	 puts(header);
	 header[53] = '\0';
	 
	 for(int i=0; i<53; i++){
	 if(i==1) 
	 	fprintf(out_img, "%c", '5');
	 else
		fprintf(out_img, "%c", header[i]);
	 printf("\t%x",header[i]);
	 }
	 puts(header);
	 header[53] = '\0';
	 unsigned int iterations = 0;
	 int dec_itr = 8;
	 
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
			//thinning_algo();
			
			//if((arr[y][x-1] == 0)&&(arr[y][x+1] == 255))
				//arr[y][x] = 0;
			//if((arr[y][x-1] == 255)&&(arr[y][x+1] == 0))
				//arr[y][x] = 0;
			
			//fprintf(out_img, "%c", arr[y][x]);
			//sigma = 0;
			//printf("%d", arr[y][x]);
				
	 }
	}
	thinning_algo();
	

#ifdef PARALLEL_THIN
	do{
	finished = true;
	for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 		thinning_algo(NORTH);
	 	}
	 	}
	 for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 		thinning_algo(SOUTH);
	 	}
	 	}
	 for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 		thinning_algo(EAST);
	 	}
	 	}
	 for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 		thinning_algo(WEST);
	 	}
	 	}
	
	}while(finished == 0);
	
#endif

	for( y=0; y<HEIGHT; y++){
	 	for( x=0; x<WIDTH; x++){
	 		fprintf(out_img, "%c", arr[y][x]);
	 	}
	 }
	return 0;
}
