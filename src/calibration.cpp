/*
Kyle Meredith 2017
Middlebury College undergraduate summer research with Daniel Scharstein

This program is adapted from calibration.cpp, camera_calibration.cpp, and stereo_calib.cpp,
which are example calibration programs provided by opencv. It supports unique
functionality with Rafael Munoz-Salinas' ArUco library, including calibration
with a 3D ArUco box rig.

The program has three modes: intrinsic calibration, stereo calibration, and live
feed preview. It supports three patterns: chessboard, ArUco single, and ArUco box.

Read the read me for more information and guidance.
*/

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <aruco.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

using namespace cv;
using namespace aruco;
using namespace std;

const char* previewHelp =
    "Preview functions:\n"
        "  <ESC>, 'q' - quit the program\n"
        "  'u' - toggle undistortion on/off\n"
        "  'c' - toggle ArUco marker coordinates/IDs\n";

//struct to store parameters for intrinsic calibration
struct intrinsicCalibration {
    Mat cameraMatrix, distCoeffs;   //intrinsic camera matrices
    vector<Mat> rvecs, tvecs;       //extrinsic rotation and translation vectors for each image
    vector<vector<Point2f> > imagePoints;   //corner points on 2d image
    vector<vector<Point3f> > objectPoints;  //corresponding 3d object points
    vector<float> reprojErrs;   //vector of reprojection errors for each pixel
    double totalAvgErr = 0;     //average error across every pixel
};

//struct to store parameters for stereo calibration
struct stereoCalibration {
    Mat R, T, E, F;         //Extrinsic matrices (rotation, translation, essential, fundamental)
    Mat R1, R2, P1, P2, Q;  //Rectification parameters (rectification transformations, projection matrices, disparity-to-depth mapping matrix)
    Rect validRoi[2];       //Rectangle within the rectified image that contains all valid points
};

//struct to store parameters for an ArUco pattern
struct arucoPattern {
    vector <MarkerMap> markerMapList;  // ArUco marker maps
    // These parameters are used to calculate the integer 3D object coordinates of the pattern (see getIntPoints)
    vector <string> planeList;      // Corresponding 3D planes for each marker map
    // The x y transformations to make the origin the bottom left corner
    int xOffset;
    int yOffset;
    int denominator;    // The denominator required to make all point values integers
};

class Settings
{
public:
    Settings() : goodInput(false) {}
    enum Pattern { CHESSBOARD, ARUCO_SINGLE, ARUCO_BOX, NOT_EXISTING };
    enum Mode { INTRINSIC, STEREO, PREVIEW, INVALID };

    //Writes settings serialization to a file. Uncomment the other write() function
    //outside the settings class to use this functionality
    void write(FileStorage& fs) const
    {
        fs << "{" << "Mode" << modeInput
                  << "Calibration_Pattern" <<  patternInput

                  << "ChessboardSize_Width"  <<  boardSize.width
                  << "ChessboardSize_Height" <<  boardSize.height
                  << "SquareSize" << squareSize

                  << "ImageList_Filename" <<  imageListFilename
                  << "ArucoConfig_Filename" <<  arucoConfigFilename
                  << "IntrinsicInput_Filename" <<  intrinsicInputFilename

                  << "IntrinsicOutput_Filename" <<  intrinsicOutput
                  << "ExtrinsicOutput_Filename" <<  extrinsicOutput

                  << "UndistortedImages_Path" <<  undistortedPath
                  << "RectifiedImages_Path" <<  rectifiedPath
                  << "DetectedImages_Path" <<  detectedPath

                  << "Calibrate_FixDistCoeffs" << fixDistCoeffs
                  << "Calibrate_FixAspectRatio" <<  aspectRatio
                  << "Calibrate_AssumeZeroTangentialDistortion" <<  assumeZeroTangentDist
                  << "Calibrate_FixPrincipalPointAtTheCenter" <<  fixPrincipalPoint

                  << "Show_UndistortedImages" <<  showUndistorted
                  << "Show_RectifiedImages" <<  showRectified
                  << "Show_ArucoMarkerCoordinates" << showArucoCoords
                  << "Wait_NextDetectedImage" << wait

                  << "LivePreviewCameraID" <<  cameraIDInput
           << "}";
    }
    void read(const FileNode& node)             //Reads settings serialization
    {
        node["Mode"] >> modeInput;
        node["Calibration_Pattern"] >> patternInput;

        node["ChessboardSize_Width" ] >> boardSize.width;
        node["ChessboardSize_Height"] >> boardSize.height;
        node["SquareSize"]  >> squareSize;

        node["ImageList_Filename"] >> imageListFilename;
        node["ArucoConfig_Filename"] >> arucoConfigFilename;
        node["IntrinsicInput_Filename"] >> intrinsicInputFilename;

        node["IntrinsicOutput_Filename"] >> intrinsicOutput;
        node["ExtrinsicOutput_Filename"] >> extrinsicOutput;

        node["UndistortedImages_Path"] >> undistortedPath;
        node["RectifiedImages_Path"] >> rectifiedPath;
        node["DetectedImages_Path"] >> detectedPath;

        node["Calibrate_FixDistCoeffs"] >> fixDistCoeffs;
        node["Calibrate_FixAspectRatio"] >> aspectRatio;
        node["Calibrate_AssumeZeroTangentialDistortion"] >> assumeZeroTangentDist;
        node["Calibrate_FixPrincipalPointAtTheCenter"] >> fixPrincipalPoint;

        node["Show_UndistortedImages"] >> showUndistorted;
        node["Show_RectifiedImages"] >> showRectified;
        node["Show_ArucoMarkerCoordinates"] >> showArucoCoords;
        node["Wait_NextDetectedImage"] >> wait;

        node["LivePreviewCameraID"] >> cameraIDInput;
        interprate();
    }
    void interprate()       //Interprets the settings and checks for valid input
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
            }
            if (!capture.isOpened())
            {
                cerr << "Invalid camera ID for live preview: " << cameraIDInput << endl;
                goodInput = false;
            }
            else
                printf( "\n%s", previewHelp );
        }
        else if (readImageList(imageListFilename))
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

        if (calibrationPattern != CHESSBOARD)       //ArUco pattern
        {
            if (readArucoConfig(arucoConfigFilename)) {
                nMarkerMaps = (int)arPat.markerMapList.size();
                if (calibrationPattern == ARUCO_SINGLE && nMarkerMaps != 1)
                {
                    cerr << "Incorrect # of marker maps for ArUco single pattern: " << nMarkerMaps << endl;
                    goodInput = false;
                }
                else if (calibrationPattern == ARUCO_BOX && nMarkerMaps != 3)
                {
                    cerr << "Incorrect # of marker maps for ArUco box pattern: " << nMarkerMaps << endl;
                    goodInput = false;
                }
            }
            else {
                cerr << "Invalid ArUco config file: " << arucoConfigFilename << endl;
                goodInput = false;
            }
        }

        useIntrinsicInput = false;
        if (readIntrinsicInput(intrinsicInputFilename)) {
            useIntrinsicInput = true;
        }
        else if (calibrationPattern == ARUCO_BOX) {
            cerr << "Must input intrinsics to calibrate with ARUCO_BOX pattern" << endl;
            goodInput = false;
        }

        flag = 0;
        int digit, shift;
        // For each '1' digit in the fixDistCoeffs setting, add the fix flag
        for (int i=0; i<5; i++)
        {
            digit = fixDistCoeffs[i] - '0';   //gets first digit as int
            // The FIX_K[1-5] flags are separated by powers of 2, with a jump of 3 after K3
            if (i >= 3) shift = i + 3;
            else shift = i;
            if (digit)
                flag |= CV_CALIB_FIX_K1 << shift;
        }

        if(fixPrincipalPoint)       flag |= CV_CALIB_FIX_PRINCIPAL_POINT;
        if(assumeZeroTangentDist)   flag |= CV_CALIB_ZERO_TANGENT_DIST;
        if(aspectRatio)             flag |= CV_CALIB_FIX_ASPECT_RATIO;
    }

    // Sets up the next image for pattern detection
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

        // If the image is too big, resize it. This makes it more visible and
        // prevents errors with ArUco detection
        if (img.cols>1280) resize(img, img, Size(), 0.5, 0.5);

        return img;
    }

    // Reads the image list from a file
    bool readImageList( const string& filename )
    {
        imageList.clear();
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() )
            return false;
        FileNode n = fs.getFirstTopLevelNode();
        if( n.type() != FileNode::SEQ )
            return false;
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it )
            imageList.push_back((string)*it);
        return true;
    }

    // Sets up arucoPattern struct from a config file
    bool readArucoConfig( const string& filename )
    {
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() )
            return false;

        FileNode n = fs["MarkerMap_Configs"];
        FileNodeIterator it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it ) {
            MarkerMap map;
            map.readFromFile((string)*it);
            arPat.markerMapList.push_back(map);
        }

        n = fs["Planes"];
        it = n.begin(), it_end = n.end();
        for( ; it != it_end; ++it ) {
            arPat.planeList.push_back((string)*it);
        }

        fs["xOffset"] >> arPat.xOffset;
        fs["yOffset"] >> arPat.yOffset;
        fs["Denominator"] >> arPat.denominator;

        return true;
    }

    // Sets up intrinsicInput struct from an intrinsics file
    bool readIntrinsicInput( const string& filename )
    {
        FileStorage fs(filename, FileStorage::READ);
        if( !fs.isOpened() ) {
            if ( filename == "0" )       // Intentional lack of input
                return false;
            else {                       // Unintentional invalid input
                cerr << "Invalid intrinsic input: " << filename << endl;
                return false;
            }
        }
        fs["Camera_Matrix"] >> intrinsicInput.cameraMatrix;
        fs["Distortion_Coefficients"] >> intrinsicInput.distCoeffs;
        return true;
    }

    // Saves the intrinsic parameters of the inCal struct to intrinsicOutput
    void saveIntrinsics(intrinsicCalibration &inCal)
    {
        if (intrinsicOutput == "0") return;
        FileStorage fs( intrinsicOutput, FileStorage::WRITE );

        time_t tm;
        time( &tm );
        struct tm *t2 = localtime( &tm );
        char buf[1024];
        strftime( buf, sizeof(buf)-1, "%c", t2 );
        fs << "Calibration_Time" << buf;

        fs << "Image_Width" << imageSize.width;
        fs << "Image_Height" << imageSize.height;

        fs << "Calibration_Pattern" << patternInput;
        if (calibrationPattern == CHESSBOARD)
        {
            fs << "Board_Width" << boardSize.width;
            fs << "Board_Height" << boardSize.height;
            fs << "Square_Size" << squareSize;
        }

        if( flag & CV_CALIB_FIX_ASPECT_RATIO )
            fs << "AspectRatio" << aspectRatio;

        if( flag )
            sprintf( buf, "%s%s%s%s%s%s%s%s%s",
                flag & CV_CALIB_FIX_K1 ? "+FIX_K1 " : "",
                flag & CV_CALIB_FIX_K2 ? "+FIX_K2 " : "",
                flag & CV_CALIB_FIX_K3 ? "+FIX_K3 " : "",
                flag & CV_CALIB_FIX_K4 ? "+FIX_K4 " : "",
                flag & CV_CALIB_FIX_K5 ? "+FIX_K5 " : "",
                flag & CV_CALIB_USE_INTRINSIC_GUESS ? "+USE_INTRINSIC_GUESS " : "",
                flag & CV_CALIB_FIX_ASPECT_RATIO ? "+FIX_ASPECT_RATIO " : "",
                flag & CV_CALIB_FIX_PRINCIPAL_POINT ? "+FIX_PRINCIPAL_POINT " : "",
                flag & CV_CALIB_ZERO_TANGENT_DIST ? "+ZERO_TANGENT_DIST " : "" );
        fs << "Calibration_Flags" << buf;

        fs << "Camera_Matrix" << inCal.cameraMatrix;
        fs << "Distortion_Coefficients" << inCal.distCoeffs;

        fs << "Avg_Reprojection_Error" << inCal.totalAvgErr;
        if( !inCal.reprojErrs.empty() )
            fs << "Per_View_Reprojection_Errors" << Mat(inCal.reprojErrs);
    }

    // Saves the stereo parameters of the sterCal struct to extrinsicOutput
    void saveExtrinsics(stereoCalibration &sterCal)
    {
        if (extrinsicOutput == "0") return;
        FileStorage fs( extrinsicOutput, FileStorage::WRITE );

        time_t tm;
        time( &tm );
        struct tm *t2 = localtime( &tm );
        char buf[1024];
        strftime( buf, sizeof(buf)-1, "%c", t2 );
        fs << "Calibration_Time" << buf;

        fs << "Calibration_Pattern" << patternInput;

        fs << "Stereo_Parameters";
        fs << "{" << "Rotation_Matrix"     << sterCal.R
                  << "Translation_Vector"  << sterCal.T
                  << "Essential_Matrix"    << sterCal.E
                  << "Fundamental_Matrix"  << sterCal.F
           << "}";

        fs << "Rectification_Parameters";
        fs << "{" << "Rectification_Transformation_1"       << sterCal.R1
                  << "Rectification_Transformation_2"       << sterCal.R2
                  << "Projection_Matrix_1"                  << sterCal.P1
                  << "Projection_Matrix_2"                  << sterCal.P2
                  << "Disparity-to-depth_Mapping_Matrix"    << sterCal.Q
           << "}";
    }

public:
//--------------------------Calibration configuration-------------------------//
    // Program modes:
    //    INTRINSIC  — calculates intrinsics parameters and  undistorts images
    //    STEREO     — calculates extrinsic stereo paramaters and rectifies images
    //    PREVIEW    — detects pattern on live feed, previewing detection and undistortion
    Mode mode;
    Pattern calibrationPattern;   // Three supported calibration patterns: CHESSBOARD, ARUCO_SINGLE, ARUCO_BOX

    Size boardSize;     // Size of chessboard (number of inner corners per chessboard row and column)
    float squareSize;   // The size of a square in some user defined metric system (pixel, millimeter, etc.)

//-----------------------------Input settings---------------------------------//
    vector<string> imageList;   // Image list to run calibration
    string imageListFilename;   // Input filename for image list

    arucoPattern arPat;      // arucoPattern struct that stores information for an ArUco pattern
    string arucoConfigFilename;      // Input filename to configure ArUco pattern

    //Intrinsic input can be used as an initial estimate for intrinsic calibration,
    //as fixed intrinsics for stereo calibration, or to preview undistortion in preview mode
    //Leave filename at "0" to calculate new intrinsics
    intrinsicCalibration intrinsicInput; // Struct to store inputted intrinsics
    string intrinsicInputFilename;       // Intrinsic input filename
    bool useIntrinsicInput;              // Boolean to simplify program

//-----------------------------Output settings--------------------------------//
    string intrinsicOutput;    // File to write results of intrinsic calibration
    string extrinsicOutput;    // File to write results of stereo calibration

    // LEAVE THESE SETTINGS AT "0" TO NOT SAVE IMAGES
    string undistortedPath;    // Path at which to save undistorted images
    string rectifiedPath;      // Path at which to save rectified images
    string detectedPath;       // Path at which to save images with detected patterns

//-----------------------Intrinsic Calibration settings-----------------------//
    // It is recommended to fix distortion coefficients 3-5 ("00111"). Only 1-2 are needed
    // in most cases, and 3 produces significant distortion in stereo rectification
    string fixDistCoeffs;         // A string of five digits (0 or 1) that control which distortion coefficients will be fixed (1 = fixed)
    float aspectRatio;            // The aspect ratio. If it is non zero, it will be fixed in calibration
    bool assumeZeroTangentDist;   // Assume zero tangential distortion
    bool fixPrincipalPoint;       // Fix the principal point at the center
    int flag;                     // Flag to modify calibration

//--------------------------------UI settings---------------------------------//
    bool showUndistorted;   // Show undistorted images after intrinsic calibration
    bool showRectified;     // Show rectified images after stereo calibration
    bool showArucoCoords;   // Draw each marker with its 3D coordinate. If false, IDs will be printed
    bool wait;              // Wait until a key is pressed to show the next detected image

//-----------------------------Program variables------------------------------//
    int nImages;        // Number of images in the image list
    Size imageSize;     // Size of each image
    int nMarkerMaps;       // Number of marker maps read from config list

//---------------------------Live Preview settings----------------------------//
    int cameraID;           //ID for live preview camera. Generally "0" is built in webcam
    VideoCapture capture;   //Live capture object

    bool goodInput;         //Tracks input validity
private:
    // Input variables only needed to set up settings
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
        // FileStorage fs("settingsOutput.yml", FileStorage::WRITE);
        // fs << "Settings" << s;

// static void write(FileStorage& fs, const string&, const Settings& x)
// {
//     x.write(fs);
// }


//----------------Error checking/Debugging helper functions-------------------//
// Checks if a path points to an existing directory
bool pathCheck(const string& path)
{
    DIR* dir = opendir(path.c_str());
    if (dir)              // If the path is an actual directory
    {
        closedir(dir);
        return true;
    }
    else                  // Directory does not exist
        return false;
}

// Legibly prints the contents of a matrix
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

// Legibly prints the points of an intrinsic calibration struct
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
// Calculates the reprojection error with a set of intrinsics
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
        inCal.reprojErrs[i] = (float)sqrt(err*err/n);
        totalErr += err*err;
        totalPoints += n;
    }
    return sqrt(totalErr/totalPoints);
}

// Calculates the 3D object points of a chessboard
void calcChessboardCorners(Settings s, vector<Point3f>& objectPointsBuf)
{
    for( int i = 0; i < s.boardSize.height; i++ )
        for( int j = 0; j < s.boardSize.width; j++ )
            objectPointsBuf.push_back(Point3f(float(j*s.squareSize),
                                      float(i*s.squareSize), 0));
}

// Calculates the 3D object points corresponding to detected ArUco markers
void calcArucoCorners(vector<Point2f> &imagePointsBuf, vector<Point3f> &objectPointsBuf,
                      const vector<Marker> &markers_detected, const MarkerMap &map)
{
    imagePointsBuf.clear();
    objectPointsBuf.clear();
    // For each detected marker
    for(size_t i=0;i<markers_detected.size();i++){
        int markerIndex = -1;
        // Search for the marker in the map
        for(size_t j=0;j<map.size() && markerIndex==-1;j++)
            if (map[j].id==markers_detected[i].id ) markerIndex=j;
        if (markerIndex != -1){
            // If the marker has been found, add its image and object points
            for(int j=0;j<4;j++){
                imagePointsBuf.push_back(markers_detected[i][j]);
                objectPointsBuf.push_back(map[markerIndex][j]);
            }
        }
    }
    //cout<<inCal.objectPoints.size()/4<<" markers detected"<<endl;
}

// Modify the object points to be integer values that correspond to 3D planes
vector<Point3f> getIntPoints(Settings s, vector<Point3f> &points, int index){
    vector<Point3f> intPoints;

    // variables to increase clarity
    string plane = s.arPat.planeList[index];
    int xOffset = s.arPat.xOffset;
    int yOffset = s.arPat.yOffset;
    int denom = s.arPat.denominator;

    if (plane == "YZ")
        for(auto p:points) intPoints.push_back(Point3f(0, (p.y + yOffset)/denom,
        (-p.x + xOffset)/denom));
    else if (plane == "XZ")
        for(auto p:points) intPoints.push_back(Point3f((p.x + xOffset)/denom, 0,
        (-p.y + yOffset)/denom));
    else   //plane == "XY"
        for(auto p:points) intPoints.push_back(Point3f((p.x + xOffset)/denom,
        (p.y + yOffset)/denom, 0));

    return intPoints;
}

// Stereo calibration requires both images to have the same # of image and object points, but
// ArUco detections can include an arbitrary subset of all markers.
// This function limits the points lists to only those points shared between each image
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

// Draws an inputted ArUco marker
// Draws either the ID or 3D coordinate, depening on the showArucoCoords setting
void drawMarker(Settings s, Marker &marker, Mat &img, Scalar color, int lineWidth, Point3f printPoint, int corner) {
    // Draw a rectangle around the marker
    // marker[x] is coordinate of corner on image
    line(img, marker[0], marker[1], color, lineWidth, CV_AA);
    line(img, marker[1], marker[2], color, lineWidth, CV_AA);
    line(img, marker[2], marker[3], color, lineWidth, CV_AA);
    line(img, marker[3], marker[0], color, lineWidth, CV_AA);

    auto p2=Point2f(lineWidth, lineWidth);
    rectangle(img, marker[corner] - p2, marker[corner] + p2,  Scalar(255 - color[0], 255 - color[1], 255 - color[2], 255), lineWidth, CV_AA);

    // Determine the center point
    Point cent(0, 0);
    for (int i = 0; i < 4; i++) {
        cent.x += marker[i].x;
        cent.y += marker[i].y;
    }
    cent.x /= 4.;
    cent.y /= 4.;

    if (s.showArucoCoords) {        // draw the input printPoint, which is the marker coordinate
        rectangle(img, marker[corner] - p2, marker[corner] + p2,  Scalar(255 - color[0], 255 - color[1], 255 - color[2], 255), lineWidth, CV_AA);

        char p[100];
        sprintf(p, "(%d,%d,%d)", (int)printPoint.x, (int)printPoint.y, (int)printPoint.z);
        putText(img, p, cent, FONT_HERSHEY_SIMPLEX, .5f, Scalar(255 - color[0], 255 - color[1], 255 - color[2], 255), 2);
    }
    else {                          // draw the ID number
        char cad[100];
        sprintf(cad, "id=%d", marker.id);
        putText(img, cad, cent, FONT_HERSHEY_SIMPLEX,  max(0.5f,float(lineWidth)*0.3f), Scalar(255 - color[0], 255 - color[1], 255 - color[2], 255), max(lineWidth,2));
    }
}

// Draws all the detected markers onto the image
void drawArucoMarkers(Settings s, Mat &img, vector<Point3f> &objectPointsBuf,
                      vector<Marker> detectedMarkers,
                      vector<int> markersFromSet, int index)
{
    // corner variable is the index of the corner to be draw
    // each marker's points are stored in a list:
    //    [upper left, upper right, lower right, lower left]
    int corner, colorSwitch;
    string plane = s.arPat.planeList[index];
    if (plane == "XY") {
        corner = 3;
        colorSwitch = 0;
    } else if (plane == "YZ") {
        corner = 2;
        colorSwitch = 1;
    } else {  //plane == "XZ"
        corner = 0;
        colorSwitch = 2;
    }

    // Color for marker to be drawn in draw function
    Scalar color = Scalar(0,0,0);
    color[colorSwitch] = 255;

    // Draws each detected markers onto the image
    // Each marker has 4 detected object points, so loop through size of inCal.objectPoints/4
    int markerIndex;
    for (int k = 0; k < (int)objectPointsBuf.size()/4; k++) {
        markerIndex = markersFromSet[k];
        drawMarker(s, detectedMarkers[markerIndex], img, color, max(float(1.f),1.5f*float(img.cols)/1000.f),
                   objectPointsBuf[k*4+corner], corner);
    }
    img.copyTo(img);
}

// Detects the pattern on a chessboard image
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

// Detects the pattern on an ArUco image
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

    // Pointers to the overall imagePoints and objectPoints vectors for the image
    // The points from all marker maps will be added to these image vectors
    vector<Point2f> *imgImagePoints;
    vector<Point3f> *imgObjectPoints;

    if (s.mode != Settings::PREVIEW) {
        imgImagePoints = &inCal.imagePoints.at(vectorIndex);
        imgObjectPoints = &inCal.objectPoints.at(vectorIndex);
    }

    //for each marker map, detect its markers and draw them
    for(int j=0; j < s.nMarkerMaps; j++) {
        MarkerMap map = s.arPat.markerMapList[j];
        TheMarkerDetector.setDictionary(map.getDictionary());
        vector<Marker> detectedMarkers;
        vector<int> markersFromSet;

        // Point buffers to store points for each config
        vector<Point2f> imagePointsBuf;
        vector<Point3f> objectPointsBuf;

        // detect the markers using MarkerDetector object
        detectedMarkers = TheMarkerDetector.detect(img);
        markersFromSet = map.getIndices(detectedMarkers);
        calcArucoCorners(imagePointsBuf,objectPointsBuf,detectedMarkers,map);

        // Convert the object points to int values. This also compensates for box geometry,
        // based on the plane list in the aruco pattern config
        objectPointsBuf = getIntPoints(s, objectPointsBuf, j);

        // Add the point buffers to the overall calibration vectors
        if(objectPointsBuf.size()>0 && s.mode != Settings::PREVIEW){
            for (auto p:imagePointsBuf) imgImagePoints->push_back(p);
            for (auto p:objectPointsBuf) imgObjectPoints->push_back(p);
        }
        drawArucoMarkers(s, img, objectPointsBuf, detectedMarkers, markersFromSet, j);
    }
}


//--------------------Running and saving functions----------------------------//
// Correct an images radial distortion using a set of intrinsic parameters
static void undistortImages(Settings s, intrinsicCalibration &inCal)
{
    Mat img, Uimg;
    char imgSave[1000];

    bool save = false;
    if(s.undistortedPath != "0" && s.mode != Settings::PREVIEW)
    {
        if( pathCheck(s.undistortedPath) )
            save = true;
        else
            printf("\nUndistorted images could not be saved. Invalid path: %s\n", s.undistortedPath.c_str());
    }

    namedWindow("Undistorted", CV_WINDOW_AUTOSIZE);
    for( int i = 0; i < s.nImages; i++ )
    {
        img = s.imageSetup(i);
        undistort(img, Uimg, inCal.cameraMatrix, inCal.distCoeffs);

        // If a valid path for undistorted images has been provided, save them to this path
        if(save)
        {
            sprintf(imgSave, "%sundistorted_%d.jpg", s.undistortedPath.c_str(), i);
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

// Rectifies an image pair using a set of extrinsic stereo parameters
void rectifyImages(Settings s, intrinsicCalibration &inCal,
                   intrinsicCalibration &inCal2, stereoCalibration &sterCal)
{
    Mat rmap[2][2];

    //Precompute maps for remap()
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
    char imgSave[1000];
    const char *view;

    bool save = false;
    if(s.rectifiedPath != "0")
    {
        if( pathCheck(s.undistortedPath) )
            save = true;
        else
            printf("\nRectified images could not be saved. Invalid path: %s\n", s.rectifiedPath.c_str());
    }

    namedWindow("Rectified", CV_WINDOW_AUTOSIZE);
    for( int i = 0; i < s.nImages/2; i++ )
    {
        for( int k = 0; k < 2; k++ )
        {
            Mat img = imread(s.imageList[i*2+k], 0), rimg, cimg;
            if (img.cols>1280) resize(img, img, Size(), 0.5, 0.5);
            remap(img, rimg, rmap[k][0], rmap[k][1], CV_INTER_LINEAR);

            // If a valid path for rectified images has been provided, save them to this path
            if (save)
            {
                view = "left";
                if (k == 1) view = "right";
                sprintf(imgSave, "%s%s_rectified_%d.jpg", s.rectifiedPath.c_str(), view, i);
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

// Run intrinsic calibration, using the image and object points to calculate the
// camera matrix and distortion coefficients
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

// Run stereo calibration, using the points and intrinsics of two viewpoints to determine
// the rotation and translation between them
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

    // Stereo calibration requires both images to have the same # of image and object points, but
    // getSharedPoints limits the points lists to only those points shared between each image
    if (s.calibrationPattern != Settings::CHESSBOARD)       //ArUco pattern
        getSharedPoints(inCal, inCal2);

    double err = stereoCalibrate(
               inCal.objectPoints, inCal.imagePoints, inCal2.imagePoints,
               inCal.cameraMatrix, inCal.distCoeffs,
               inCal2.cameraMatrix, inCal2.distCoeffs,
               s.imageSize, sterCal.R, sterCal.T, sterCal.E, sterCal.F, TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 1000, 1e-10), CV_CALIB_FIX_INTRINSIC);

    printf("\nStereo reprojection error = %.4f\n", err);

    // Rectify the images using these extrinsic results
    stereoRectify(inCal.cameraMatrix, inCal.distCoeffs,
                 inCal2.cameraMatrix, inCal2.distCoeffs,
                 s.imageSize, sterCal.R, sterCal.T, sterCal.R1, sterCal.R2,
                 sterCal.P1, sterCal.P2, sterCal.Q,
                 CALIB_ZERO_DISPARITY, 1, s.imageSize,
                 &sterCal.validRoi[0], &sterCal.validRoi[1]);

    rectifyImages(s, inCal, inCal2, sterCal);
    return sterCal;
}

// Runs the appropriate calibration based on the mode and saves the results
void runCalibrationAndSave(Settings s, intrinsicCalibration &inCal, intrinsicCalibration &inCal2)
{
    bool ok;
    if (s.mode == Settings::STEREO) {         // stereo calibration
        if (!s.useIntrinsicInput)
        {
        ok = runIntrinsicCalibration(s, inCal);
        printf("%s for left. Avg reprojection error = %.4f\n",
                ok ? "\nIntrinsic calibration succeeded" : "\nIntrinsic calibration failed",
                inCal.totalAvgErr);
        ok = runIntrinsicCalibration(s, inCal2);
        printf("%s for right. Avg reprojection error = %.4f\n",
                ok ? "\nIntrinsic calibration succeeded" : "\nIntrinsic calibration failed",
                inCal2.totalAvgErr);
        } else
            ok = true;

        stereoCalibration sterCal = runStereoCalibration(s, inCal, inCal2);
        s.saveExtrinsics(sterCal);

    } else {                        // intrinsic calibration
        ok = runIntrinsicCalibration(s, inCal);
        printf("%s. Avg reprojection error = %.4f\n",
                ok ? "\nIntrinsic calibration succeeded" : "\nIntrinsic calibration failed",
                inCal.totalAvgErr);

        if( ok ) {
            undistortImages(s, inCal);
            s.saveIntrinsics(inCal);
        }
    }
}

// Undistorts the preview image if the setting has been toggled with the 'u' key
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
            cerr << "\nUndistorted preview requires intrinsic input.\n";
            undistortPreview = !undistortPreview;
        }
    }
}

// Main function. Detects patterns on images, runs calibration and saves results
int calibrateWithSettings( const string inputSettingsFile )
{
    Settings s;
    FileStorage fs(inputSettingsFile, FileStorage::READ);   // Read the settings
    if (!fs.isOpened())
    {
        cerr << "Could not open the settings file: \"" << inputSettingsFile << "\"" << endl;
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
    intrinsicCalibration *currentInCal = &inCal;

    int size = (s.mode == Settings::STEREO) ? s.nImages/2 : s.nImages;
    //ArUco patterns require correctly sized vectors
    if (s.calibrationPattern != Settings::CHESSBOARD) {
        inCal.imagePoints.resize(size);
        inCal.objectPoints.resize(size);
        inCal2.imagePoints.resize(size);
        inCal2.objectPoints.resize(size);
    }
    int vectorIndex = -1;
    bool undistortPreview = false;

    char imgSave[1000];
    bool save = false;
    if(s.detectedPath != "0" && s.mode != Settings::PREVIEW)
    {
        if( pathCheck(s.detectedPath) )
            save = true;
        else
            printf("\nDetected images could not be saved. Invalid path: %s\n", s.detectedPath.c_str());
    }

    namedWindow("Detected", CV_WINDOW_AUTOSIZE);
    // For each image in the image list
    for(int i = 0;;i++)
    {
        // Switches between intrinsic calibration structs for stereo mode
        if (i%2 == 0) {
            currentInCal = &inCal;
            vectorIndex++;
        } else if (s.mode == Settings::STEREO)
            currentInCal = &inCal2;
        else
            vectorIndex++;

        // Set up the image
        Mat img = s.imageSetup(i);

        // If there is no data, the photos have run out
        if(!img.data)
        {
            if((int)inCal.imagePoints.size() > 0) {
                destroyWindow("Detected");
                runCalibrationAndSave(s, inCal, inCal2);
            }
            break;
        }
        s.imageSize = img.size();

        //Detect the pattern in the image, adding data to the imagePoints
        //and objectPoints calibration parameters
        if(s.calibrationPattern == Settings::CHESSBOARD)
            chessboardDetect(s, img, *currentInCal);
        else
            arucoDetect(s, img, *currentInCal, vectorIndex);

        if (s.mode == Settings::PREVIEW)    // Check if the preview should be undistorted
            undistortCheck(s, img, undistortPreview);

        // If a valid path for detected images has been provided, save them to this path
        if(save)
        {
            sprintf(imgSave, "%sdetected_%d.jpg", s.detectedPath.c_str(), i);
            imwrite(imgSave, img);
        }

        imshow("Detected", img);

        // If wait setting is true, wait till next key press (waitkey(0)). Otherwise, wait 50 ms
        char c = (char)waitKey(s.wait ? 0: 50);

        if (c == 'u')
            undistortPreview = !undistortPreview;
        if (c == 'c' && s.mode == Settings::PREVIEW)
            s.showArucoCoords = !s.showArucoCoords;
        else if( (c & 255) == 27 || c == 'q' || c == 'Q' )
            break;
    }
    destroyWindow("Detected");
    return 0;
}
