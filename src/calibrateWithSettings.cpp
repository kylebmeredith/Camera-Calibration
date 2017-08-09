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
    const char * inputSettingsFile;
    if (argc != 2) {
        cerr << "Usage: calibrateWithSettings [path to settings file]" << endl
             << "The settings folder contains several example files with "
                "descriptions of each parameter. Check the README for more detail." << endl;
        return -1;
    }
    else {
        inputSettingsFile = argv[1];
        calibrateWithSettings(inputSettingsFile);
    }
}
