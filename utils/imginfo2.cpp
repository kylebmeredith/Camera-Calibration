/* imginfo.cpp - prints information about an image
 * also demonstrates conversions to gray and other types
 *
 * CS 453 openCV demo
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

// print information about image
void printinfo(Mat img)
{
    printf("width=%d, height=%d, channels=%d, pixel type: ",
	   img.cols, img.rows, img.channels());
    switch(img.depth()) {
    case CV_8U:	 printf("CV_8U  -  8-bit unsigned int (0..255)\n"); break;
    case CV_8S:  printf("CV_8S  -  8-bit signed int (-128..127)\n"); break;
    case CV_16U: printf("CV_16U - 16-bit unsigned int (0..65535)\n"); break;
    case CV_16S: printf("CV_16S - 16-bit signed int (-32768..32767)\n"); break;
    case CV_32S: printf("CV_32S - 32-bit signed int\n"); break;
    case CV_32F: printf("CV_32F - 32-bit float\n"); break;
    case CV_64F: printf("CV_64F - 64-bit double\n"); break;
    default:     printf("unknown\n");
    }
}


int main(int argc, char **argv)
{
    if (argc < 2) {
	fprintf(stderr, "usage: %s img\n", argv[0]);
	exit(1);
    }

    Mat img = imread(argv[1], -1); // color; include alpha channel if present
    if (img.empty()) { 
	fprintf(stderr, "cannot read image %s\n", argv[1]); 
	exit(1); 
    }

    printinfo(img);

    if (1) { // demonstrate conversions
	Mat img2;
	printf("converting to gray\n");
	cvtColor(img, img, CV_BGR2GRAY);
	printinfo(img);

	Mat img3;
	printf("converting to float image\n");
	img.convertTo(img, CV_32F);
	printinfo(img);

	printf("converting to uchar image\n");
	img.convertTo(img, CV_8U);
	printinfo(img);

	printf("converting to color\n");
	cvtColor(img, img, CV_GRAY2BGR);
	printinfo(img);

	printf("writing as test.png\n");
	imwrite("test.png", img);
	
    }

    return 0;
}
