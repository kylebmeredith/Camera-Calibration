/* canny.cpp - canny edges
 *
 * CS 453 openCV demo
 *
 * OpenCV can only run canny on 8U (byte) images, so it won't work for
 * smoothing with high sigmas...
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

Mat im0, ims, ime;
const char *wins = "Smoothed";
const char *wine = "Edges";
int sigmaval = 0;
int loval = 0;
int hival = 0;

void mycanny()
{
    //printf("lo = %d   hi = %d\n", loval, hival);
    Canny(ims, ime, loval, hival, 3, true);
    // invert black to white
    ime.convertTo(ime, -1, -1, 255); // rtype=-1 (same result type), scale=-1, offset=255
    imshow(wine, ime);
}

void changelo(int, void *)
{
    if (hival < loval)
	setTrackbarPos("hi", wine, loval);
    mycanny();
}

void changehi(int, void *)
{
    if (loval > hival)
	setTrackbarPos("lo", wine, hival);
    mycanny();
}


void mysmooth(int, void *)
{
    float sigma = sigmaval / 10.0;
    int i = ((int)sigma)*4 + 1;
    printf("sigma = %.2f   i = %d\n", sigma, i);
    GaussianBlur(im0, ims, Size(i, i), sigma, sigma);
    Mat imb;
    ims.convertTo(imb, CV_8U);
    imshow(wins, imb);
    mycanny();
}

int main(int argc, char ** argv)
{
    if (argc < 2) {
	fprintf(stderr, "usage: %s imagefile\n", argv[0]);
	exit(1);
    }

    im0 = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    if (im0.empty()) { 
	fprintf(stderr, "cannot read image %s\n", argv[1]); 
	exit(1); 
    }
    //im0.convertTo(im0, CV_32F);
    namedWindow(wins, CV_WINDOW_AUTOSIZE);
    namedWindow(wine, CV_WINDOW_AUTOSIZE);
    createTrackbar("10sigma", wins, &sigmaval, 100, mysmooth);
    createTrackbar("lo", wine, &loval, 40, changelo);
    createTrackbar("hi", wine, &hival, 40, changehi);
    mysmooth(0, 0);
    waitKey(0);

    return 0;
}
