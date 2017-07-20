/* readwrite.cpp - reading and writing images
 *
 * CS 453 openCV demo
 */

#include <stdio.h>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    if (argc < 3) {
	fprintf(stderr, "usage: %s inimg outimg\n", argv[0]);
	exit(1);
    }

    Mat img = imread(argv[1], 1); // second arg: 1-color (3-band), 0-gray
    if (img.empty()) { 
	fprintf(stderr, "cannot read image %s\n", argv[1]); 
	exit(1); 
    }

    imwrite(argv[2], img);

    return 0;
}
