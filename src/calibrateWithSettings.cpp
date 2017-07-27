#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "aruco.h"

using namespace std;
using namespace cv;
using namespace aruco;

#include "calibration.h"

int main( int argc, char** argv )
{
    const char * inputSettingsFile = argc > 1 ? argv[1] : "settings.yml";
    calibrateWithSettings(inputSettingsFile);
}
