/* showvideo.cpp - display video in window
 *
 * CS 453 openCV demo
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

const char *win = "video";

int main()
{
    int cam = 0; // default camera
    VideoCapture cap(cam);
    if (!cap.isOpened()) {
	fprintf(stderr, "cannot open camera %d\n", cam);
	exit(1);
    }

    namedWindow(win, CV_WINDOW_AUTOSIZE);
    Mat frame;
    for (int i=0; i<10; i++) {
      while (1) {
        cap >> frame;
        imshow(win, frame);
        if (waitKey(30) >= 0) // wait up to 30 msec
	  break;
      }
      char fname[1000];
      sprintf(fname, "frame%d.png", i);
      imwrite(fname, frame);
    }

    return 0;
}
