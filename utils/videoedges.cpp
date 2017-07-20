/* videoedges.cpp - detect edges in video
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

    //namedWindow(win);
    namedWindow(win, 0);
    Mat frame, edges;
    while (1) {
        cap >> frame;
        cvtColor(frame, edges, CV_BGR2GRAY);
        GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
        Canny(edges, edges, 15, 30, 3);
        imshow(win, edges);
        if (waitKey(30) >= 0) // wait up to 30 msec
	    break;
    }
    
    return 0;
}
