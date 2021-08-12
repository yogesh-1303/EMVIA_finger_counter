
#include <stdio.h>
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>

#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/video/video.hpp"

#define WIDTH 	640
#define HEIGHT 	480
#define NSEC_PER_SEC	1000000000	
#define NUM_THREADS 8

using namespace cv;
using namespace std;

typedef struct
{
    int threadIdx;
} threadParams_t;

pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_mutex_t lock;

Mat frame[NUM_THREADS], gray_frame[NUM_THREADS], thresh_frame[NUM_THREADS];
String dev_name;

long double sum_rate = 0, max_time = 0, min_time = NSEC_PER_SEC;

struct timespec frame_start_time, frame_stop_time;
int framecnt=0;



//Does the frame rate analysis with min, max, current, avg framte rates and jitter produced
//takes and returns no arguments
void frame_analysis(){

	long double time_diff=0;
	
		framecnt++;
	
		if(frame_stop_time.tv_sec == frame_start_time.tv_sec)                   //If the second has not changed
    time_diff = ((long double) frame_stop_time.tv_nsec - (long double) frame_start_time.tv_nsec)/NSEC_PER_SEC;
   
    //if the seconds field has changed from start time to stop time
    else{
        //when the nano seconds of start time > that of stop time (e.g., start = 2s 80ns; stop = 3s 40ns)
         if (frame_start_time.tv_nsec > frame_stop_time.tv_nsec)	{
		time_diff = ((((long double) frame_stop_time.tv_sec - (long double) frame_start_time.tv_sec - 1) * NSEC_PER_SEC)
                            + (NSEC_PER_SEC - ((long double) frame_start_time.tv_nsec - (long double) frame_stop_time.tv_nsec)))/NSEC_PER_SEC;
		}
        //when nano seconds of stop time > that of start time (e.g., start = 2s 500ns; stop = 4s 600ns)
        else 
        time_diff = ((((long double) frame_stop_time.tv_sec - (long double) frame_start_time.tv_sec) * NSEC_PER_SEC) + 
                                ((long double) frame_stop_time.tv_nsec - (long double) frame_start_time.tv_nsec))/NSEC_PER_SEC;
        }                        
    
    //calculating the max and min time taken by a frame
    if(time_diff > max_time)
           max_time = time_diff;
    if(time_diff < min_time)
            min_time = time_diff;
    //sum of rates
        sum_rate += (1/time_diff);
        
        syslog(LOG_USER, "\nFrame #%d   time taken = %Lf \n current frame rate = %Lf Hz, max frame rate = %Lf Hz,  worst-case frame rate = %Lf Hz, avg rate = %Lf Hz, jitter = %Lf Hz",
                                framecnt, time_diff, 1/time_diff, 1/min_time, 1/max_time, sum_rate/(long double)framecnt, (1/min_time)-(1/max_time));
}

void *frame_proc(void *threadp){
	
	threadParams_t *threadParams = (threadParams_t *)threadp;
	Mat erode_frame[NUM_THREADS], dilate_frame[NUM_THREADS], sub_frame[NUM_THREADS], canny_frame[NUM_THREADS]; 
	Mat kernel = getStructuringElement(MORPH_CROSS, Size(3,3));
	Mat kernel2 = getStructuringElement(MORPH_CROSS, Size(5,5));
	int done, iterations;
	vector<Vec4i> lines;
	clock_gettime(CLOCK_REALTIME, &frame_start_time);
	//pthread_mutex_lock(&lock);
	cvtColor(frame[threadParams->threadIdx], gray_frame[threadParams->threadIdx], CV_BGR2GRAY);
	//Threshold the image using inRange function of openCV
		cvtColor(frame[threadParams->threadIdx], frame[threadParams->threadIdx], CV_BGR2HSV);
		inRange(frame[threadParams->threadIdx], Scalar(0, 10, 60), Scalar(25, 150, 255), thresh_frame[threadParams->threadIdx]);
		for(int j=1; j<=15; j+=2)
		medianBlur(thresh_frame[threadParams->threadIdx], thresh_frame[threadParams->threadIdx], j);
		Mat skel(thresh_frame[threadParams->threadIdx].size(), CV_8UC1, Scalar(0));
		iterations = 0;
		//do the morphological operation for skeletonization using repeated erosion and dilation
		do{
		erode(thresh_frame[threadParams->threadIdx], erode_frame[threadParams->threadIdx], kernel, Point(-1,-1), 1);
		dilate(erode_frame[threadParams->threadIdx], dilate_frame[threadParams->threadIdx], kernel2, Point(-1,-1), 1);
		subtract(thresh_frame[threadParams->threadIdx], dilate_frame[threadParams->threadIdx], sub_frame[threadParams->threadIdx]);
		bitwise_or(skel, sub_frame[threadParams->threadIdx], skel);
		erode_frame[threadParams->threadIdx].copyTo(thresh_frame[threadParams->threadIdx]);
		done = (countNonZero(thresh_frame[threadParams->threadIdx])==0);
		iterations++;
		}while((!done)&&(iterations<100));
		
	//imshow("gray frame", thresh_frame[threadParams->threadIdx]);
	HoughLinesP(skel, lines, 1, CV_PI/180, 50, 10, 20 );

    	for( size_t i = 0; i < lines.size(); i++ )
    	{        	
    		line( frame[threadParams->threadIdx], Point(lines[i][0], lines[i][1]),
        	Point( lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
    	}
    	//pthread_mutex_unlock(&lock);
    clock_gettime(CLOCK_REALTIME, &frame_stop_time);
		imshow("skeleton", skel);
		imshow("HSV frame", frame[threadParams->threadIdx]);
		waitKey(5);
	frame_analysis();
	
}

int main(int argc, char **argv)
{
    if(argc > 2)
        dev_name = argv[1];
    else
        dev_name = "/dev/video0";

	String finger_count = "finger count image window";

    // Create window
    namedWindow( finger_count, CV_WINDOW_AUTOSIZE );
    
    //defining the cap object for capture from camera
	VideoCapture cap;
	cap.open(0);
	if(!cap.isOpened()){
		printf("Error opening video\n");
		exit(-1);
	}
	
	//Mat erode_frame, dilate_frame, sub_frame, canny_frame; 
	//Mat kernel = getStructuringElement(MORPH_CROSS, Size(3,3));
	//Mat kernel2 = getStructuringElement(MORPH_CROSS, Size(5,5));

	int frame_num=0;
	//int done, iterations;
	
	vector<Point2f> corners;
	
	while(1){
		for(int k=0; k<NUM_THREADS; k++){
		cap.read(frame[k]);
		
		threadParams[k].threadIdx=k;

       if(pthread_create(&threads[k],   // pointer to thread descriptor
                      NULL,     // use default attributes
                      frame_proc, // thread function entry point
                      (void *)&(threadParams[k]) // parameters to pass in
                     )!=0) printf("Not Created");
		//if(frame[k].empty()) break;
		//imshow("thresholded frame", frame[k]);	
		
		}
		for(int k=0;k<NUM_THREADS;k++)
       pthread_join(threads[k], NULL);
		/*
		clock_gettime(CLOCK_REALTIME, &frame_start_time);
		//cvtColor(frame, gray_frame, CV_BGR2GRAY);
		//threshold(gray_frame, thresh_frame, 130, 255, CV_THRESH_BINARY_INV);
		//Threshold the image using inRange function of openCV
		cvtColor(frame, frame, CV_BGR2HSV);
		inRange(frame, Scalar(0, 10, 60), Scalar(25, 150, 255), thresh_frame);
		//inRange(frame, Scalar(0, 58, 30), Scalar(33, 150, 255), thresh_frame);
		
		//repeated median blurring for noise reduction with increasing kernel sizes
		for(int j=1; j<=15; j+=2)
		medianBlur(thresh_frame, thresh_frame, j);
		Mat skel(thresh_frame.size(), CV_8UC1, Scalar(0));
		iterations = 0;
	
		//do the morphological operation for skeletonization using repeated erosion and dilation
		do{
		erode(thresh_frame, erode_frame, kernel, Point(-1,-1), 1);
		dilate(erode_frame, dilate_frame, kernel2, Point(-1,-1), 1);
		subtract(thresh_frame, dilate_frame, sub_frame);
		bitwise_or(skel, sub_frame, skel);
		erode_frame.copyTo(thresh_frame);
		done = (countNonZero(thresh_frame)==0);
		iterations++;
		}while((!done)&&(iterations<100)); 

		int num_line=0;
		//detecting and drawing lines on the skeletal image and counting them
		
		HoughLinesP(skel, lines, 1, CV_PI/180, 50, 10, 20 );

    	for( size_t i = 0; i < lines.size(); i++ )
    	{        	
    		line( frame, Point(lines[i][0], lines[i][1]),
        	Point( lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
    	}
    
    	clock_gettime(CLOCK_REALTIME, &frame_stop_time);
    	//cout<<"\nframe num: "<<frame_num<<"\tlines = "<<lines.size();//<<"\tnumber of lines = "<<num_line;
    	const String fingers = format ("%d", lines.size()); 
   		putText(frame, fingers, Point(5,50),FONT_HERSHEY_SIMPLEX, 2,  Scalar(255,0,0), 2, LINE_AA, false);
   		
    	//storing the images for later conversion into video
		String filename = format("arm_skel%d.jpg",frame_num);
		//imwrite(filename, skel);
		String filename2 = format("arm_HSV%d.jpg",frame_num);
		imwrite(filename2, frame);
		frame_num++;

		//displaying the frames as video
		for(int k=1; k<=4; k++){
		imshow("thresholded frame", frame[k]);	
		//imshow(finger_count, skel);
		waitKey(5);
		}
		//frame_analysis();
*/		
	}

return 0;
}


