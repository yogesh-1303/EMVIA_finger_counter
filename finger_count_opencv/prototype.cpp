

#include <stdio.h>
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

#include <time.h>

#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/videoio/videoio.hpp"
#include "opencv2/video/video.hpp"

//#define HARRIS

using namespace cv;
using namespace std;

Mat frame;
int thresh=130;
String dev_name;

unsigned int i=0;

int main(int argc, char **argv)
{

	//for passing the threshold value as runtime argument
	char *a;

	if (argc > 1)
		 a = argv[1];
		thresh = atoi(a);

	if(argc > 2)
        dev_name = argv[2];
    else
        dev_name = "/dev/video0";

	String timg_window_name = "prototype image window";




  /// Create window
  namedWindow( timg_window_name, CV_WINDOW_AUTOSIZE );

	//defining frames to be used
	Mat bgframe, erode_frame, blur_frame, dilate_frame, sub_frame;

	//getting the kernel structuring element of size 3x3
   Mat kernel = getStructuringElement(MORPH_CROSS, Size(3,3));

	int iterations = 0;
	bool done;

	unsigned int j=0;

//defining the cap object for capture from camera
		VideoCapture cap;
	cap.open(0);
	if(!cap.isOpened()){
		printf("Error opening video\n");
		exit(-1);
	}


		struct timespec frame_start_time, frame_stop_time;
	while(j!=3000){

		//read the video frame by frame
		cap.read(frame);
		if(frame.empty()) break;
		clock_gettime(CLOCK_REALTIME, &frame_start_time);

		//convert to graymap and then binary map using thresholding
		cvtColor(frame, frame, COLOR_RGB2GRAY, 1);
		threshold(frame, blur_frame, thresh, 255, THRESH_BINARY
		);
		//median blur for noice suppression
		medianBlur(blur_frame, blur_frame, 3);
		//define a zero frame of size same as blur_frame
		Mat skel(blur_frame.size(), CV_8UC1, Scalar(0));

		//perform erosion and dilation on the blurred frame and then
		//subtract the initial blurred frame from the dilated one
		//do bitwie or with the zero frame, store the result in the zero
		//frame and make eroded frame the new blurred frame.
		do{

		erode(blur_frame, erode_frame, kernel, Point(-1,-1), 1,
				 BORDER_CONSTANT);

		dilate(erode_frame, dilate_frame, kernel, Point(-1,-1), 1,
				 	BORDER_CONSTANT);

		subtract(blur_frame, dilate_frame, sub_frame);

		bitwise_or(skel, sub_frame, skel);
		erode_frame.copyTo(blur_frame);

		//done if the number of non-zero pixels are 0
		done = (countNonZero(blur_frame)==0);
		iterations++;

		}while((!done)&&(iterations<100));
	 int corners=0, sum=0, x;

//if using Harris for corner detection
#ifdef HARRIS
		Mat dst = Mat::zeros(blur_frame.size(), CV_32FC1);
		cornerHarris(skel, dst, 2, 3, 0.04);

    Mat dst_norm, dst_norm_scaled;
    normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );
    convertScaleAbs( dst_norm, dst_norm_scaled );
    for( int i = 0; i < dst_norm.rows ; i++ )
    {
        for( int j = 0; j < dst_norm.cols; j++ )
        {
            if( (int) dst_norm.at<float>(i,j) > thresh )
            {
                circle( dst_norm_scaled, Point(j,i), 5,  Scalar(0), 2, 8, 0 );
                corners++;
            }
        }
    }
#else
//if using the custom algorithm for endpoint detection
//the endpoint has only one active neighbor(value = 255)
//hence if the sum of all neighbors is 255, it is a corner
	for(int i=1;i<320;i++){
		for(int j=1; j<240; j++){
			x = (int) skel.at<uchar>(i,j);
			if(x == 0)
				continue;
			else{
			//claculating sum in a neighborhood
			sum = skel.at<uchar>(i,j+1) + skel.at<uchar>(i-1,j+1) +
					skel.at<uchar>(i-1,j) + skel.at<uchar>(i-1,j-1) +
					skel.at<uchar>(i,j-1) + skel.at<uchar>(i+1,j-1) +
					skel.at<uchar>(i+1,j) + skel.at<uchar>(i+1,j+1);
			if(sum == 255)
				corners++;
		}

		}
	}
#endif
	clock_gettime(CLOCK_REALTIME, &frame_stop_time);
	syslog(LOG_USER, "time: %ld microsec", (frame_stop_time.tv_nsec -
								 frame_start_time.tv_nsec)/1000);

    printf("\nCorners = %d", corners);
    imshow(timg_window_name, skel);
	waitKey(5);
#ifdef HARRIS
    String corner_window = "Corners";
    namedWindow(corner_window);
    imshow( corner_window, dst_norm_scaled );
	String filename = format("hand%d.jpg",j);
	imwrite(filename, dst_norm_scaled);
	j++;
	waitKey(5);
#endif
}
    return 0;
}
