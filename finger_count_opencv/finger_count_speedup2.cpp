
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

Mat frame;
String dev_name;

long double sum_rate = 0, max_time = 0, min_time = NSEC_PER_SEC;

struct timespec frame_start_time, frame_stop_time;
int framecnt=0;

//Does the frame rate analysis with min, max, current, avg framte rates and jitter produced
//takes and returns no arguments
void frame_analysis(){

	long double time_diff=0;
	
		framecnt++;
	
	//If the second has not changed	
	if(frame_stop_time.tv_sec == frame_start_time.tv_sec)
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

Mat kernel = getStructuringElement(MORPH_CROSS, Size(3,3));
Mat kernel2 = getStructuringElement(MORPH_CROSS, Size(5,5));
vector<Mat> v;
Mat dest(frame.size(), CV_8UC1);

int itr=0;

//callback for a thread processing the frames
void *thread_proc(void *threadp){
	
	int done, iterations;
	vector<Vec4i> lines;
	threadParams_t *threadParams = (threadParams_t *)threadp;	
	Rect roi;
	roi.x = 0;
	roi.y = (frame.rows/8)*(threadParams->threadIdx);
	roi.width = frame.cols-1;
	roi.height = (frame.rows/8)-1;
	
	Mat seg(frame, roi);
	
	//Threshold the image using inRange function of openCV
	cvtColor(seg, seg, CV_BGR2HSV);
	inRange(seg, Scalar(0, 10, 60), Scalar(25, 150, 255), seg);
	Mat blur_seg(seg.size(), CV_8UC1);
	for(int j=1; j<=15; j+=2)
		medianBlur(seg, blur_seg, j);
	Mat skel(blur_seg.size(), CV_8UC1, Scalar(0));
	Mat dilate_seg(blur_seg.size(), CV_8UC1);
	Mat erode_seg(blur_seg.size(), CV_8UC1);
	Mat sub_seg(blur_seg.size(), CV_8UC1);
	
	iterations = 0;

	//do the morphological operation for skeletonization using repeated erosion and dilation
	do{
		erode(blur_seg, erode_seg, kernel, Point(-1,-1), 1);
		dilate(erode_seg, dilate_seg, kernel2, Point(-1,-1), 1);
		subtract(blur_seg, dilate_seg, sub_seg);
		bitwise_or(skel, sub_seg, skel);
		erode_seg.copyTo(blur_seg);
		done = (countNonZero(blur_seg)==0);
		iterations++;
	}while((!done)&&(iterations<100)); 
		
	//HoughLines transformation to draw lines on skeletonized frame thread
	HoughLinesP(skel, lines, 1, CV_PI/180, 50, 10, 20 );
	for( size_t i = 0; i < lines.size(); i++ )
    	{
    		line(seg, Point(lines[i][0], lines[i][1]),
        	Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
    	}

	imwrite(format("final_skel_img%d_%d.jpg", itr, threadParams->threadIdx), skel);
	imwrite(format("final_seg_img%d_%d.jpg", itr, threadParams->threadIdx), seg);
	
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
    
     if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        exit(-1);
    }
    
    //defining the cap object for capture from camera
	VideoCapture cap;
	cap.open(0);
	if(!cap.isOpened()){
		printf("Error opening video\n");
		exit(-1);
	}
	
	Mat erode_frame, dilate_frame, sub_frame, thresh_frame, canny_frame, gray_frame; 

	int frame_num=0;
	
	while(1){
		cap.read(frame);
		if(frame.empty()) break;


		clock_gettime(CLOCK_REALTIME, &frame_start_time);
		for(int i=0; i < NUM_THREADS; i++)
   		{
       		threadParams[i].threadIdx=i;

       		if(pthread_create(&threads[i],   		// pointer to thread descriptor
                      NULL,     			 		// use default attributes
                      thread_proc, 			 		// thread function entry point
                      (void *)&(threadParams[i]) 	// parameters to pass in
                     )!=0) printf("Not Created");
   		}
 
    for(int i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);
    
	clock_gettime(CLOCK_REALTIME, &frame_stop_time);
	itr++;	
	}

return 0;
}


