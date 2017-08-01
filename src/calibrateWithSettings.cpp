#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdio.h>
#include <string.h>
#include <time.h>

using namespace std;
using namespace cv;
using namespace aruco;

#include "calibration.h"

int main( int argc, char** argv )
{
    const char * inputSettingsFile = argc > 1 ? argv[1] : "settings.yml";
    calibrateWithSettings(inputSettingsFile);
}
