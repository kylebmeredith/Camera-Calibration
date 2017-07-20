/* smooth.cpp - image smoothing using track bars
 *
 * CS 453 openCV demo
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

// global variables
Mat src, dst; 
const char *win = "Smoothing";
int sigmaval = 0;
int boxval = 0;

// Gaussian smoothing
void mysmooth(int, void *)
{
    float sigma = sigmaval / 4.0;
    int i = ((int)sigma)*4 + 1;
    printf("sigma = %.2f   i = %d\n", sigma, i);
    GaussianBlur(src, dst, Size(i, i), sigma, sigma);
    imshow(win, dst);
}

// box filter smoothing
void mybox(int, void *)
{
    int i = 2*boxval+1;
    printf("box filter,  i = %d\n", i);
    blur(src, dst, Size(i, i), Point(-1,-1));
    imshow(win, dst);
}


int main(int argc, char ** argv)
{
    if (argc < 2) {
	fprintf(stderr, "usage: %s imagefile\n", argv[0]);
	exit(1);
    }

    src = imread(argv[1], 1);
    if (!src.data) { 
	fprintf(stderr, "cannot read image %s\n", argv[1]); 
	exit(1); 
    }

    namedWindow(win, CV_WINDOW_AUTOSIZE);
    createTrackbar("sigma", win, &sigmaval, 100, mysmooth);
    createTrackbar("box", win, &boxval, 40, mybox);
    imshow(win, src);
    waitKey(0);

    return 0;
}
