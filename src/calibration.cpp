/*****************************
Copyright 2011 Rafael Muñoz Salinas. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Rafael Muñoz Salinas.
********************************/
/*
Kyle Meredith

This program is adapted from calibration.cpp, camera_calibration.cpp (opencv examples)
and aruco_calibration_fromimages. Its primary function is to calibrate a camera
based on images of a chessboard, single aruco markermap, or aruco box setup
It also supports a live feed preview of what the program will recognize
*/

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
#include <aruco.h>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

using namespace cv;
using namespace aruco;
using namespace std;

//struct to store parameters for intrinsic calibration
struct intrinsicCalibration {
    Mat cameraMatrix, distCoeffs;   //intrinsic matrices
    vector<Mat> rvecs, tvecs;       //extrinsic rotation and translation vectors
    vector<vector<Point2f> > imagePoints;   //corner points on 2d image
    vector<vector<Point3f> > objectPoints;  //corresponding 3d object points
    vector<float> reprojErrs;   //vector of reprojection errors for each pixel
    double totalAvgErr = 0;     //average error across every pixel
};

//struct to store parameters for stereo calibration
struct stereoCalibration {
    Mat R, T, E, F;         //Extrinsic matrices (rotation, translation, essential, fundamental)
    Mat R1, R2, P1, P2, Q;  //Rectification parameters ()
    Rect validRoi[2];       //Rectangle within the rectified image that contains all valid points
};

class Settings
{
public:
    Settings() : goodInput(false) {}
    enum Pattern { CHESSBOARD, ARUCO_SINGLE, ARUCO_BOX, NOT_EXISTING };
    enum Mode { INTRINSIC, STEREO, PREVIEW, INVALID };

    void write(FileStorage& fs) const       //Write serialization for this class
    {
        fs << "{" << "Mode" << modeInput
                  << "Calibration_Pattern" <<  patternInput

                  << "ChessboardSize_Width"  <<  boardSize.width
                  << "ChessboardSize_Height" <<  boardSize.height
                  << "SquareSize" << squareSize

                  << "imageList_Filename" <<  imageListFilename
                  << "arucoConfigList_Filename" <<  configListFilename
                  << "intrinsicInput_Filename" <<  intrinsicInputFilename

                  << "IntrinsicOutput_Filename" <<  intrinsicOutput
                  << "UndistortedImages_Path" <<  undistortedPath

                  << "ExtrinsicOutput_Filename" <<  extrinsicOutput
                  << "RectifiedImages_Path" <<  rectifiedPath

                  << "Calibrate_FixDistCoeffs" << fixDistCoeffs
                  << "Calibrate_FixAspectRatio" <<  aspectRatio
                  << "Calibrate_AssumeZeroTangentialDistortion" <<  assumeZeroTangentDist
                  << "Calibrate_FixPrincipalPointAtTheCenter" <<  fixPrincipalPoint

                  << "Show_UndistortedImages" <<  showUndistorted
                  << "Show_RectifiedImages" <<  showRectified
                  << "Show_ArucoMarkerCoordinates" << showArucoCoords

                  << "LivePreviewCameraID" <<  cameraIDInput
           << "}";
    }
    void read(const FileNode& node)                          //Read serialization for this class
    {
        node["Mode"] >> modeInput;
        node["Calibration_Pattern"] >> patternInput;

        node["ChessboardSize_Width" ] >> boardSize.width;
        node["ChessboardSize_Height"] >> boardSize.height;
        node["SquareSize"]  >> squareSize;

        node["imageList_Filename"] >> imageListFilename;
        node["arucoConfigList_Filename"] >> configListFilename;
        node["intrinsicInput_Filename"] >> intrinsicInputFilename;

        node["IntrinsicOutput_Filename"] >> intrinsicOutput;
        node["ExtrinsicOutput_Filename"] >> extrinsicOutput;

        node["UndistortedImages_Path"] >> undistortedPath;
        node["RectifiedImages_Path"] >> rectifiedPath;

        node["Calibrate_FixDistCoeffs"] >> fixDistCoeffs;
        node["Calibrate_FixAspectRatio"] >> aspectRatio;
        node["Calibrate_AssumeZeroTangentialDistortion"] >> assumeZeroTangentDist;
        node["Calibrate_FixPrincipalPointAtTheCenter"] >> fixPrincipalPoint;

        node["Show_UndistortedImages"] >> showUndistorted;
        node["Show_UndistortedImages"] >> showRectified;
        node["Show_ArucoMarkerCoordinates"] >> showArucoCoords;

        node["LivePreviewCameraID"] >> cameraIDInput;
        interprate();
    }
    void interprate()
    {
        goodInput = true;

        mode = INVALID;
        if (!modeInput.compare("INTRINSIC")) mode = INTRINSIC;
        if (!modeInput.compare("STEREO")) mode = STEREO;
        if (!modeInput.compare("PREVIEW")) mode = PREVIEW;
        if (mode == INVALID)
            {
                cerr << "Invalid calibration mode: " << modeInput << endl;
                goodInput = false;
            }

        calibrationPattern = NOT_EXISTING;
        if (!patternInput.compare("CHESSBOARD")) calibrationPattern = CHESSBOARD;
        if (!patternInput.compare("ARUCO_SINGLE")) calibrationPattern = ARUCO_SINGLE;
        if (!patternInput.compare("ARUCO_BOX")) calibrationPattern = ARUCO_BOX;
        if (calibrationPattern == NOT_EXISTING)
            {
                cerr << "Invalid calibration pattern: " << patternInput << endl;
                goodInput = false;
            }

        if (boardSize.width <= 0 || boardSize.height <= 0)
        {
            cerr << "Invalid chessboard size: " << boardSize.width << " " << boardSize.height << endl;
            goodInput = false;
        }
        if (squareSize <= 10e-6)
        {
            cerr << "Invalid square size " << squareSize << endl;
            goodInput = false;
        }

        if (mode == PREVIEW)
        {
            if (cameraIDInput[0] >= '0' && cameraIDInput[0] <= '9')
            {
                stringstream ss(cameraIDInput);
                ss >> cameraID;
                capture.open(cameraID);
            } else {
                cerr << "Invalid camera ID for live preview: " << cameraIDInput << endl;
                goodInput = false;
            }
        }
        else if (readStringList(imageListFilename, imageList))
        {
            nImages = (int)imageList.size();
            if (mode == STEREO)
                if (nImages % 2 != 0) {
                    cerr << "Image list must have even # of elements for stereo calibration" << endl;
                    goodInput = false;
                }
        }
        else {
            cerr << "Invalid image list: " << imageListFilename << endl;
            goodInput = false;
        }

        if (calibrationPattern != CHESSBOARD)       //Aruco pattern
        {
            if (readConfigList(configListFilename, configList)) {
                nConfigs = (int)configList.size();
                if (calibrationPattern == ARUCO_SINGLE && nConfigs != 1)
                {
                    cerr << "Incorrect # of configs for single aruco pattern: " << nConfigs << endl;
                    goodInput = false;
                }
                else if (calibrationPattern == ARUCO_BOX && nConfigs != 3)
                {
                    cerr << "Incorrect # of configs for aruco box rig: " << nConfigs << endl;
                    goodInput = false;
                }
            }
            else {
                cerr << "Invalid aruco config list: " << configListFilename << endl;
                goodInput = false;
            }
        }

        useIntrinsicInput = false;
        if (readIntrinsicInput(intrinsicInputFilename, intrinsicInput)) {
            useIntrinsicInput = true;
        }
        else if (calibrationPattern == ARUCO_BOX) {
            cerr << "Must input intrinsics to calibrate with ARUCO_BOX pattern" << endl;
            goodInput = false;
        }

        flag = 0;
        int digit, shift;
        for (int i=0; i<5; i++)
        {
            digit = fixDistCoeffs[i] - '0';     //gets first digit as int
            if (i >= 3)
                shift = i + 3;
            else
                shift = i;
            if (digit) {
                flag |= CV_CALIB_FIX_K1 << shift;
            }
        }

        if(fixPrincipalPoint)       flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(assumeZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)             flag |= CV_CALIB_FIX_ASPECT_RATIO;
    }

    Mat imageSetup(int imageIndex)
    {
        Mat img;
        if( capture.isOpened() )
        {
            Mat capImg;
            capture >> capImg;
            capImg.copyTo(img);
        }
        else if( imageIndex < (int)imageList.size() )
            img = imread(imageList[imageIndex], CV_LOAD_IMAGE_COLOR);

        // if the image is too big, resize it
        if (img.cols>1280) resize(img, img, Size(), 0.25, 0.25);

        return img;
    }

    static bool readStringList( const string& filename, vector<string>& l )
    {
        l.clear();
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() )
            return false;
        FileNode n = fs.getFirstTopLevelNode();
        if( n.type() != FileNode::SEQ )
            return false;
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it )
            l.push_back((string)*it);
        return true;
    }

    static bool readConfigList( const string& filename, vector<MarkerMap>& l )
    {
        l.resize(0);
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() )
            return false;
        FileNode n = fs.getFirstTopLevelNode();
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it ) {
            MarkerMap config;
            config.readFromFile((string)*it);
            l.push_back(config);
        }
        return true;
    }

    static bool readIntrinsicInput( const string& filename, intrinsicCalibration& intrinsicInput )
    {
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() ) {
            if ( !filename.compare("0") )       // Intentional lack of input
                return false;
            else {                              // Unintentional invalid input
                cerr << "Invalid intrinsic input: " << filename << endl;
                return false;
            }
        }
        fs["camera_matrix"] >> intrinsicInput.cameraMatrix;
        fs["distortion_coefficients"] >> intrinsicInput.distCoeffs;
        return true;
    }

    static bool fileCheck( const string& filename, FileStorage fs, const char * var)
    {
        if( !fs.isOpened() ) {
            if ( !filename.compare("0") )    // Intentional lack of input if it = "0"
                return false;
            else {                           // Unintentional invalid input
                printf("Invalid %s: %s\n", var, filename.c_str());
                return false;
            }
        } else {
            return true;
        }
    }

    void saveIntrinsics(intrinsicCalibration &inCal)
    {
        FileStorage fs( intrinsicOutput, FileStorage::WRITE );
        if (!fileCheck( intrinsicOutput, fs, "intrinsic output"))
            return;

        time_t tm;
        time( &tm );
        struct tm *t2 = localtime( &tm );
        char buf[1024];
        strftime( buf, sizeof(buf)-1, "%c", t2 );
        fs << "calibration_Time" << buf;

        fs << "image_width" << imageSize.width;
        fs << "image_height" << imageSize.height;

        fs << "calibration_pattern" << patternInput;
        if (calibrationPattern == CHESSBOARD)
        {
            fs << "board_width" << boardSize.width;
            fs << "board_height" << boardSize.height;
            fs << "square_size" << squareSize;
        }

        if( flag & CV_CALIB_FIX_ASPECT_RATIO )
            fs << "aspectRatio" << aspectRatio;

        if( flag )
        {
            sprintf( buf, "%s%s%s%s%s%s%s%s%s",
                flag & CV_CALIB_FIX_K1 ? "+fix_k1 " : "",
                flag & CV_CALIB_FIX_K2 ? "+fix_k2 " : "",
                flag & CV_CALIB_FIX_K3 ? "+fix_k3 " : "",
                flag & CV_CALIB_FIX_K4 ? "+fix_k4 " : "",
                flag & CV_CALIB_FIX_K5 ? "+fix_k5 " : "",
                flag & CV_CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess " : "",
                flag & CV_CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio " : "",
                flag & CV_CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point " : "",
                flag & CV_CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist " : "" );
            // cvWriteComment( *fs, buf, 0 );
        }
        fs << "calibration_flags" << buf;
        fs << "flagValue" << flag;

        fs << "camera_matrix" << inCal.cameraMatrix;
        fs << "distortion_coefficients" << inCal.distCoeffs;

        fs << "avg_reprojection_error" << inCal.totalAvgErr;
        if( !inCal.reprojErrs.empty() )
            fs << "per_view_reprojection_errors" << Mat(inCal.reprojErrs);
    }

    void saveExtrinsics(stereoCalibration &sterCal)
    {
        FileStorage fs( extrinsicOutput, FileStorage::WRITE );
        if (!fileCheck(extrinsicOutput, fs, "extrinsic output"))
            return;

        time_t tm;
        time( &tm );
        struct tm *t2 = localtime( &tm );
        char buf[1024];
        strftime( buf, sizeof(buf)-1, "%c", t2 );
        fs << "calibration_Time" << buf;

        fs << "calibration_pattern" << patternInput;

        fs << "Rotation_Matrix"     << sterCal.R;
        fs << "Translation_Vector"  << sterCal.T;
        fs << "Essential_Matrix"    << sterCal.E;
        fs << "Fundamental_Matrix"  << sterCal.F;

        // fs << "Rectification" << "Rectification_Transformation_1"   << sterCal.R1
        //                       << "Rectification_Transformation_2"   << sterCal.R2
        //                       << "Projection_Matrix_1"  << sterCal.P1
        //                       << "Projection_Matrix_2"  << sterCal.P2
        //                       << "Disparity-to-depth_Mapping_Matrix"  << sterCal.Q;


        fs << "Rectification_Transformation_1"   << sterCal.R1;
        fs << "Rectification_Transformation_2"   << sterCal.R2;
        fs << "Projection_Matrix_1"  << sterCal.P1;
        fs << "Projection_Matrix_2"  << sterCal.P2;
        fs << "Disparity-to-depth_Mapping_Matrix"  << sterCal.Q;
    }

public:
//--------------------------Calibration configuration-------------------------//
    Mode mode;
    Pattern calibrationPattern;   // One of the Chessboard, circles, or asymmetric circle pattern

    Size boardSize;                 // The size of the board -> Number of items by width and height
    float squareSize;               // The size of a square in your defined unit (point, millimeter,etc).

//-----------------------------Input settings---------------------------------//
    vector<string> imageList;
    string imageListFilename;

    vector <MarkerMap> configList; // Aruco config files
    string configListFilename;    // Input filename for aruco config files

    intrinsicCalibration intrinsicInput; // Struct to store inputted intrinsics
    string intrinsicInputFilename;    // Leave it at 0 to calculate new intrinsics
    bool useIntrinsicInput;

//-----------------------------Output settings--------------------------------//
    string intrinsicOutput;    // File to write results of intrinsic calibration
    string extrinsicOutput;    // File to write extrisics of stereo calibration

    string undistortedPath;    // Path at which to save undistorted images (leave "0" to not save undistorted)
    string rectifiedPath;      // Path at which to save rectified images (leave "0" to not save rectified)

//-----------------------Intrinsic Calibration settings-----------------------//
    // It is recommended to fix distortion coefficients 3-5 ("00111"). Only 1-2 are needed,
    // and 3 produces significant distortion in stereo rectification
    string fixDistCoeffs;           // A string of five digits (0 or 1) that control which distortion coefficients will be fixed
    float aspectRatio;              // The aspect ratio. If it is non zero, it will be fixed in calibration
    bool assumeZeroTangentDist;     // Assume zero tangential distortion
    bool fixPrincipalPoint;         // Fix the principal point at the center
    int flag;                       // Flag to modify calibration

//--------------------------------UI settings---------------------------------//
    bool showUndistorted;           // Show undistorted images after intrinsic calibration
    bool showRectified;           // Show rectified images after stereo calibration
    bool showArucoCoords;           // If false, IDs will be printed instead

//-----------------------------Program variables------------------------------//
    int nImages;
    Size imageSize;
    int nConfigs;

//---------------------------Live Preview settings----------------------------//
    int cameraID;
    VideoCapture capture;

    bool goodInput;
private:
    string modeInput;
    string patternInput;
    string cameraIDInput;
};

static void read(const FileNode& node, Settings& x, const Settings& default_value = Settings())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

// Uncomment write() if you want to save your settings, using code like this:
//
// FileStorage fs("settingsOutput.yml", FileStorage::WRITE);
// fs << "Settings" << s;

// static void write(FileStorage& fs, const string&, const Settings& x)
// {
//     x.write(fs);
// }

const char* liveCaptureHelp =
    "When the live video from camera is used as input, the following hot-keys may be used:\n"
        "  <ESC>, 'q' - quit the program\n"
        "  'u' - switch undistortion on/off\n";

//-----------------------Debugging helper functions---------------------------//
void printMat(Mat m, const char *name)
{
    Size s = m.size();
    printf("%s: \t[", name);
    for (int i=0; i < s.height; i++)
    {
        for (int j=0; j < s.width; j++)
            printf("%.2f, ", m.at<double>(i,j));
        // if(i+1 != s.height)
            // cout << endl << "\t ";
    }
    cout << "]\n\n";
}

void printPoints(const intrinsicCalibration inCal)
{
    for (auto v:inCal.objectPoints)
    {
        cout << "object " << v.size() << endl << "[";
        for (auto p:v)
             cout << " " << p << " ";
        cout << endl << endl;
    }
    for (auto v:inCal.imagePoints)
    {
        cout << "image " << v.size() << endl << "[";
        for (auto p:v)
             cout << " " << p << " ";
        cout << endl << endl;
    }
}


//-------------------------Calibration functions------------------------------//
double computeReprojectionErrors(intrinsicCalibration &inCal)
{
    vector<Point2f> imagePoints2;
    int i, totalPoints = 0;
    double totalErr = 0, err;
    inCal.reprojErrs.resize(inCal.objectPoints.size());
    for( i = 0; i < (int)inCal.objectPoints.size(); i++ )
    {
        projectPoints(Mat(inCal.objectPoints[i]), inCal.rvecs[i], inCal.tvecs[i],
                      inCal.cameraMatrix, inCal.distCoeffs, imagePoints2);
        err = norm(Mat(inCal.imagePoints[i]), Mat(imagePoints2), CV_L2);
        int n = (int)inCal.objectPoints[i].size();
        inCal.reprojErrs[i] = (float)std::sqrt(err*err/n);
        totalErr += err*err;
        totalPoints += n;
    }
    return std::sqrt(totalErr/totalPoints);
}

void calcChessboardCorners(Settings s, vector<Point3f>& objectPointsBuf)
{
    for( int i = 0; i < s.boardSize.height; i++ )
        for( int j = 0; j < s.boardSize.width; j++ )
            objectPointsBuf.push_back(Point3f(float(j*s.squareSize),
                                      float(i*s.squareSize), 0));
}

//given the set of aruco markers detected, determine the corresponding 3d object points
void calcArucoCorners(vector<Point2f> &imagePointsBuf, vector<Point3f> &objectPointsBuf,
                            const vector<Marker> &markers_detected,
                            const MarkerMap &map)
{
    imagePointsBuf.clear();
    objectPointsBuf.clear();
    //for each detected marker
    for(size_t i=0;i<markers_detected.size();i++){
        int markerIndex=-1;
        //find the marker in the map
        for(size_t j=0;j<map.size() && markerIndex==-1;j++)
            if (map[j].id==markers_detected[i].id ) markerIndex=j;
        if (markerIndex!=-1){
            for(int j=0;j<4;j++){
                imagePointsBuf.push_back(markers_detected[i][j]);
                objectPointsBuf.push_back(map[markerIndex][j]);
            }
        }
    }
    //cout<<inCal.objectPoints.size()/4<<" markers detected"<<endl;
}

vector<Point3f> getIntPoints(vector<Point3f> &points, int plane){
    vector<Point3f> intPoints;
    // GENERALIZE THIS
    switch (plane) {
        case 0:   // XY plane
            for(auto p:points) intPoints.push_back(Point3f((p.x+1000)/125, (p.y+1000)/125, 0));
            break;
        case 1:   // YZ plane
            for(auto p:points) intPoints.push_back(Point3f(0, (p.y+1000)/125, (-p.x+1000)/125));
            break;
        case 2:   // XZ plane
            for(auto p:points) intPoints.push_back(Point3f((p.x+1000)/125, 0, (-p.y+1000)/125));
            break;
    }
    return intPoints;
}

void getSharedPoints(intrinsicCalibration &inCal, intrinsicCalibration &inCal2)
{
    // pointers to make code more legible
    vector<Point3f> *oPoints, *oPoints2;
    vector<Point2f> *iPoints, *iPoints2;
    int shared;     //index of a shared object point

    //for each objectPoints vector in overall objectPoints vector of vectors
    for (int i=0; i<(int)inCal.objectPoints.size(); i++)
    {
        vector<Point3f> sharedObjectPoints;
        vector<Point2f> sharedImagePoints, sharedImagePoints2;   //shared image points for each inCal

        oPoints = &inCal.objectPoints.at(i);
        oPoints2 = &inCal2.objectPoints.at(i);
        iPoints  = &inCal.imagePoints.at(i);
        iPoints2 = &inCal2.imagePoints.at(i);
        for (int j=0; j<(int)oPoints->size(); j++)
        {
            for (shared=0; shared<(int)oPoints2->size(); shared++)
                if (oPoints->at(j) == oPoints2->at(shared)) break;
            if (shared != (int)oPoints2->size())       //object point is shared
            {
                sharedObjectPoints.push_back(oPoints->at(j));
                sharedImagePoints.push_back(iPoints->at(j));
                sharedImagePoints2.push_back(iPoints2->at(shared));
            }
        }
        *oPoints = sharedObjectPoints;
        *oPoints2 = sharedObjectPoints;
        *iPoints = sharedImagePoints;
        *iPoints2 = sharedImagePoints2;
    }
}

void drawArucoMarkers(Settings s, Mat &img, vector<Point3f> &objectPointsBuf,
                      vector<Marker> detectedMarkers,
                      vector<int> markersFromSet, int index)
{
    // corner is the index of the corner to be draw
    // each marker's points are stored in a list:
    //    [upper left, upper right, lower right, lower left]
    int corner;
    switch (index) {
        case 0:
            corner = 3;
            break;
        case 1:
            corner = 2;
            break;
        case 2:
            corner = 0;
            break;
        default:
            corner = 3;
    }
    // Color for marker to be drawn in draw function
    Scalar color = Scalar(0,0,0);
    color[index] = 255;

    // Draws each detected markers onto the image
    // Each marker has 4 detected object points, so loop through size of inCal.objectPoints/4
    int markerIndex;
    for (int k = 0; k < (int)objectPointsBuf.size()/4; k++) {
        markerIndex = markersFromSet[k];
        detectedMarkers[markerIndex].draw(img, color, std::max(float(1.f),1.5f*float(img.cols)/1000.f),
                                          objectPointsBuf[k*4+corner], !s.showArucoCoords, s.showArucoCoords, corner);
    }
    img.copyTo(img);
}

void chessboardDetect(Settings s, Mat &img, intrinsicCalibration &inCal)
{
    //create grayscale copy for cornerSubPix function
    Mat imgGray;
    cvtColor(img, imgGray, COLOR_BGR2GRAY);

    //buffer to store points for each image
    vector<Point2f> imagePointsBuf;
    vector<Point3f> objectPointsBuf;
    bool found = findChessboardCorners( img, s.boardSize, imagePointsBuf,
        CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS | CV_CALIB_CB_FAST_CHECK |
        CV_CALIB_CB_NORMALIZE_IMAGE);
    if (found)
    {
        cornerSubPix(imgGray, imagePointsBuf, Size(11,11), Size(-1,-1),
                     TermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));

        //add these image points to the overall calibration vector
        inCal.imagePoints.push_back(imagePointsBuf);

        //find the corresponding objectPoints
        calcChessboardCorners(s, objectPointsBuf);
        inCal.objectPoints.push_back(objectPointsBuf);
        drawChessboardCorners(img, s.boardSize, Mat(imagePointsBuf), found);
    }
}

void arucoDetect(Settings s, Mat &img, intrinsicCalibration &inCal, int vectorIndex)
{
    MarkerDetector TheMarkerDetector;
    //set specific parameters for this configuration
    MarkerDetector::Params params;
    params._borderDistThres=.01;//acept markers near the borders
    params._maxSize=0.9;
    params._thresParam1=5;
    params._thresParam1_range=10;//search in wide range of values for param1
    params._cornerMethod=MarkerDetector::SUBPIX;//use subpixel corner refinement
    params._subpix_wsize= (10./2000.)*float(img.cols) ;//search corner subpix in a window area
    //cout<<params._subpix_wsize<<" "<<float(img.cols)<<endl;
    TheMarkerDetector.setParams(params);//set the params above

    //pointers to the overall imagePoints and objectPoints vectors for the image
    //the points from all three config maps will be added to the image vectors

    vector<Point2f> *imgImagePoints;
    vector<Point3f> *imgObjectPoints;

    if (s.mode != Settings::PREVIEW) {
        imgImagePoints = &inCal.imagePoints.at(vectorIndex);
        imgObjectPoints = &inCal.objectPoints.at(vectorIndex);
    }

    //for each config file, detect its markers and draw them
    for(int j=0; j < s.nConfigs; j++) {
        MarkerMap MarkerMapConfig = s.configList[j];
        TheMarkerDetector.setDictionary(MarkerMapConfig.getDictionary());
        vector<Marker> detectedMarkers;
        vector<int> markersFromSet;

        //point buffers to store points for each config
        vector<Point2f> imagePointsBuf;
        vector<Point3f> objectPointsBuf;

        // detect the markers using MarkerDetector object
        detectedMarkers = TheMarkerDetector.detect(img);
        markersFromSet = MarkerMapConfig.getIndices(detectedMarkers);
        calcArucoCorners(imagePointsBuf,objectPointsBuf,detectedMarkers,MarkerMapConfig);

        // Convert the object points to int values. This also compensates for box geometry;
        // box plane is based on config's index value j (xy, yz, xz)
        // Because of this, our aruco box config list file must be in order 3, 2, 1
        objectPointsBuf = getIntPoints(objectPointsBuf, j);

        // add the point buffers to the overall calibration vectors
        if(objectPointsBuf.size()>0 && s.mode != Settings::PREVIEW){
            for (auto p:imagePointsBuf) imgImagePoints->push_back(p);
            for (auto p:objectPointsBuf) imgObjectPoints->push_back(p);
        }
        drawArucoMarkers(s, img, objectPointsBuf, detectedMarkers, markersFromSet, j);
    }
}


//--------------------Running and saving functions----------------------------//

static void undistortImages(Settings s, intrinsicCalibration &inCal)
{
    Mat img, Uimg;
    char imgSave[30];
    for( int i = 0; i < s.nImages; i++ )
    {
        img = s.imageSetup(i);
        undistort(img, Uimg, inCal.cameraMatrix, inCal.distCoeffs);

        if(s.undistortedPath.compare("0") != 0  &&  s.mode != Settings::PREVIEW)
        {
            sprintf(imgSave, "%sundistorted_%d.png", s.undistortedPath.c_str(), i);
            imwrite(imgSave, Uimg);
        }

        if(s.showUndistorted)
        {
            imshow("Undistorted", Uimg);
            char c = (char)waitKey();
            if( (c & 255) == 27 || c == 'q' || c == 'Q' )   //escape key or 'q'
                break;
        }
    }
    destroyWindow("Undistorted");
}

void rectifyImages(Settings s, intrinsicCalibration &inCal,
                   intrinsicCalibration &inCal2, stereoCalibration &sterCal)
{
    Mat rmap[2][2];

    //Precompute maps for cv::remap()
    initUndistortRectifyMap(inCal.cameraMatrix, inCal.distCoeffs, sterCal.R1,
                        sterCal.P1, s.imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);
    initUndistortRectifyMap(inCal2.cameraMatrix, inCal2.distCoeffs, sterCal.R2,
                        sterCal.P2, s.imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);

    Mat canvas, rimg, cimg;
    double sf = 600. / MAX(s.imageSize.width, s.imageSize.height);
    int w = cvRound(s.imageSize.width * sf);
    int h = cvRound(s.imageSize.height * sf);
    canvas.create(h, w*2, CV_8UC3);

    // buffer for image filename
    char imgSave[30];
    const char *view;

    for( int i = 0; i < s.nImages/2; i++ )
    {
        for( int k = 0; k < 2; k++ )
        {
            Mat img = imread(s.imageList[i*2+k], 0), rimg, cimg;
            if (img.cols>1280) resize(img, img, Size(), 0.5, 0.5);
            remap(img, rimg, rmap[k][0], rmap[k][1], CV_INTER_LINEAR);

            // if a path for rectified images has been provided, save them to this path
            if (s.rectifiedPath.compare("0") != 0)
            {
                view = "left";
                if (k == 1) view = "right";
                sprintf(imgSave, "%s%s_rect_%d.png", s.rectifiedPath.c_str(), view, i);
                imwrite(imgSave, rimg);
            }

            cvtColor(rimg, cimg, COLOR_GRAY2BGR);
            Mat canvasPart = canvas(Rect(w*k, 0, w, h));
            resize(cimg, canvasPart, canvasPart.size(), 0, 0, CV_INTER_AREA);

            Rect vroi(cvRound(sterCal.validRoi[k].x*sf), cvRound(sterCal.validRoi[k].y*sf),
                      cvRound(sterCal.validRoi[k].width*sf), cvRound(sterCal.validRoi[k].height*sf));
            rectangle(canvasPart, vroi, Scalar(0,0,255), 3, 8);
        }
        for( int j = 0; j < canvas.rows; j += 16 )
            line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
        if (s.showRectified)
        {
            imshow("Rectified", canvas);
            char c = (char)waitKey();
            if( c == 27 || c == 'q' || c == 'Q' )
                break;
        }
    }
    destroyWindow("Rectified");
}

bool runIntrinsicCalibration(Settings s, intrinsicCalibration &inCal)
{
    if (s.useIntrinsicInput)     //precalculated intrinsic have been inputted. Use these
    {
        inCal.cameraMatrix = s.intrinsicInput.cameraMatrix;
        inCal.distCoeffs = s.intrinsicInput.distCoeffs;
        calibrateCamera(inCal.objectPoints, inCal.imagePoints, s.imageSize,
                        inCal.cameraMatrix, inCal.distCoeffs,
                        inCal.rvecs, inCal.tvecs, s.flag | CV_CALIB_USE_INTRINSIC_GUESS);

    } else {                //else, create empty matrices to be calculated
        inCal.cameraMatrix = Mat::eye(3, 3, CV_64F);
        inCal.distCoeffs = Mat::zeros(8, 1, CV_64F);

        calibrateCamera(inCal.objectPoints, inCal.imagePoints, s.imageSize,
                        inCal.cameraMatrix, inCal.distCoeffs,
                        inCal.rvecs, inCal.tvecs, s.flag);
    }

    // if( flag & CV_CALIB_FIX_ASPECT_RATIO )
    //     inCal.cameraMatrix.at<double>(0,0) = aspectRatio;

    bool ok = checkRange(inCal.cameraMatrix) && checkRange(inCal.distCoeffs);
    inCal.totalAvgErr = computeReprojectionErrors(inCal);
    return ok;
}


stereoCalibration runStereoCalibration(Settings s, intrinsicCalibration &inCal, intrinsicCalibration &inCal2)
{
    stereoCalibration sterCal;
    if (s.useIntrinsicInput)     //precalculated intrinsic have been inputted. Use these
    {
        inCal.cameraMatrix = s.intrinsicInput.cameraMatrix;
        inCal2.cameraMatrix = s.intrinsicInput.cameraMatrix;
        inCal.distCoeffs = s.intrinsicInput.distCoeffs;
        inCal2.distCoeffs = s.intrinsicInput.distCoeffs;
    }

    if (s.calibrationPattern != Settings::CHESSBOARD)       //aruco pattern
        getSharedPoints(inCal, inCal2);

    double err = stereoCalibrate(
               inCal.objectPoints, inCal.imagePoints, inCal2.imagePoints,
               inCal.cameraMatrix, inCal.distCoeffs,
               inCal2.cameraMatrix, inCal2.distCoeffs,
               s.imageSize, sterCal.R, sterCal.T, sterCal.E, sterCal.F,
               TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, 1e-10),
               CV_CALIB_FIX_INTRINSIC);

    printf("Stereo reprojection error = %.4f\n", err);

    stereoRectify(inCal.cameraMatrix, inCal.distCoeffs,
                 inCal2.cameraMatrix, inCal2.distCoeffs,
                 s.imageSize, sterCal.R, sterCal.T, sterCal.R1, sterCal.R2,
                 sterCal.P1, sterCal.P2, sterCal.Q,
                 CALIB_ZERO_DISPARITY, 1, s.imageSize,
                 &sterCal.validRoi[0], &sterCal.validRoi[1]);

    rectifyImages(s, inCal, inCal2, sterCal);
    return sterCal;
}

static void undistortCheck(Settings s, Mat &img, bool &undistortPreview)
{
    if (undistortPreview)
    {
        if (s.useIntrinsicInput)
        {
            Mat temp = img.clone();
            undistort(temp, img, s.intrinsicInput.cameraMatrix,
                      s.intrinsicInput.distCoeffs);
        } else {
            cerr << "Undistorted preview requires intrinsic input";
            undistortPreview = !undistortPreview;
        }
    }
}

bool runCalibrationAndSave(Settings s, intrinsicCalibration &inCal, intrinsicCalibration &inCal2)
{
    bool ok;
    if (s.mode == Settings::STEREO) {         // stereo calibration
        if (!s.useIntrinsicInput)
        {
        ok = runIntrinsicCalibration(s, inCal);
        printf("%s for left. Avg reprojection error = %.4f\n",
                ok ? "Intrinsic calibration succeeded" : "Intrinsic calibration failed",
                inCal.totalAvgErr);
        ok = runIntrinsicCalibration(s, inCal2);
        printf("%s for right. Avg reprojection error = %.4f\n",
                ok ? "Intrinsic calibration succeeded" : "Intrinsic calibration failed",
                inCal2.totalAvgErr);
        } else
            ok = true;

        stereoCalibration sterCal = runStereoCalibration(s, inCal, inCal2);

        s.saveExtrinsics(sterCal);

    } else {                        // intrinsic calibration
        ok = runIntrinsicCalibration(s, inCal);
        printf("%s. Avg reprojection error = %.4f\n",
                ok ? "Intrinsic calibration succeeded" : "Intrinsic calibration failed",
                inCal.totalAvgErr);

        if( ok ) {
            undistortImages(s, inCal);
            s.saveIntrinsics(inCal);
        }
    }
    return ok;
}


int calibrateWithSettings( const string inputSettingsFile )
{
    Settings s;
    FileStorage fs(inputSettingsFile, FileStorage::READ); // Read the settings
    if (!fs.isOpened())
    {
        cerr << "Could not open the configuration file: \"" << inputSettingsFile << "\"" << endl;
        return -1;
    }
    fs["Settings"] >> s;
    fs.release();                                         // close Settings file

    if (!s.goodInput)
    {
        cerr << "Invalid input detected. Application stopping. " << endl;
        return -1;
    }

    //struct to store calibration parameters
    intrinsicCalibration inCal, inCal2;
    int size = (s.mode == Settings::STEREO) ? s.nImages/2 : s.nImages;

    if (s.calibrationPattern != Settings::CHESSBOARD) {
        inCal.imagePoints.resize(size);
        inCal.objectPoints.resize(size);
        inCal2.imagePoints.resize(size);
        inCal2.objectPoints.resize(size);
    }

    intrinsicCalibration *currentInCal = &inCal;

    // CHECK THIS FOR RESOLUTION
    namedWindow( "Image View", CV_WINDOW_AUTOSIZE );

    int vectorIndex = -1;
    bool undistortPreview = false;

    // For each image in the image list
    for(int i = 0;;i++)
    {
        //this is unecessarily complicated, but it works
        if (i%2 == 0) {
            currentInCal = &inCal;
            vectorIndex++;
        } else if (s.mode == Settings::STEREO)
            currentInCal = &inCal2;
        else
            vectorIndex++;

        //set up the view
        Mat img = s.imageSetup(i);

        // if there is no data, the capture has closed or the photos have run out
        if(!img.data)
        {
            if((int)inCal.imagePoints.size() > 0)
                runCalibrationAndSave(s, inCal, inCal2);
            break;
        }
        s.imageSize = img.size();

        //Detect the pattern in the image, adding data to the imagePoints
        //and objectPoints calibration parameters
        if(s.calibrationPattern == Settings::CHESSBOARD)
            chessboardDetect(s, img, *currentInCal);
        else
            arucoDetect(s, img, *currentInCal, vectorIndex);

        if (s.mode == Settings::PREVIEW)
            undistortCheck(s, img, undistortPreview);

        imshow("Image View", img);

        // char c = waitKey();
        char c = waitKey(s.capture.isOpened() ? 50 : 100);

        if (c == 'u')
            undistortPreview = !undistortPreview;
        else if( (c & 255) == 27 || c == 'q' || c == 'Q' )
            break;
    }
    cerr << "wat";
    destroyWindow("Image View");
    return 0;
}
