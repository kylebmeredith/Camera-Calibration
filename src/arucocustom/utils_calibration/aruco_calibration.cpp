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
#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "aruco.h"
#include "aruco_calibration_board_a4.h"
using namespace cv;
using namespace aruco;

float TheMarkerSize = -1;
VideoCapture TheVideoCapturer;
Mat TheInputImage, TheInputImageCopy;
CameraParameters TheCameraParameters;
MarkerMap TheMarkerMapConfig;
MarkerDetector TheMarkerDetector;

string TheOutCameraParams;

void cvTackBarEvents(int pos, void *);
  int iThresParam1, iThresParam2;
int waitTime = 10;

class CmdLineParser{int argc; char **argv; public:  CmdLineParser(int _argc,char **_argv):argc(_argc),argv(_argv){}  bool operator[] ( string param ) {int idx=-1;  for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i;    return ( idx!=-1 ) ;    } string operator()(string param,string defvalue="-1"){int idx=-1;    for ( int i=0; i<argc && idx==-1; i++ ) if ( string ( argv[i] ) ==param ) idx=i; if ( idx==-1 ) return defvalue;   else  return ( argv[  idx+1] ); }};


//given the set of markers detected, the function determines the get the 2d-3d correspondes
void getMarker2d_3d(vector<cv::Point2f> &p2d, vector<cv::Point3f> &p3d, const vector< Marker >&markers_detected,const MarkerMap &bc){
    p2d.clear();
    p3d.clear();
    //for each detected marker
    for(size_t i=0;i<markers_detected.size();i++){
        //find it in the bc
        int fidx=-1;
        for(size_t j=0;j<bc.size() &&fidx==-1;j++)
            if (bc[j].id==markers_detected[i].id ) fidx=j;
        if (fidx!=-1){
            for(int j=0;j<4;j++){
                p2d.push_back(markers_detected[i][j]);
                p3d.push_back(bc[fidx][j]);
            }
        }
    }
 cout<<"points added"<<endl;
}
vector < vector<cv::Point2f>  > calib_p2d;
vector < vector<cv::Point3f>  > calib_p3d;
aruco::CameraParameters camp;//camera parameters estimated


/************************************
 *
 *
 *
 *
 ************************************/
int main(int argc, char **argv) {
    try {
        CmdLineParser cml(argc,argv);
        if (argc <3 || cml["-h"]){
            cerr << "Usage: (in.avi|live) out_camera_calibration.yml  [-m markermapConfig.yml (configuration of the board. If use default one (in utils), no need to set this)]    [-size <float> :(value in meters of a marker. If you provide a board that contains that information, this is ommited) ] " << endl;
            return -1;
        }
        // parse arguments
        //load marker info from file if indicated
        if (cml["-m"]) TheMarkerMapConfig.readFromFile( cml("-m"));
        else {//use default
            stringstream sstr;sstr.write((char*)default_a4_board,default_a4_board_size);
            TheMarkerMapConfig.fromStream(sstr);
        }
        if (!TheMarkerMapConfig.isExpressedInMeters() && !cml["-size"]){
            cerr<<"Need to specify the size of the makers (in meters) with -size"<<endl;
            return -1;
        }
        if (!TheMarkerMapConfig.isExpressedInMeters())
            TheMarkerMapConfig=TheMarkerMapConfig.convertToMeters(atof(cml("-size").c_str()));
        // read from camera or from  file
        if (string(argv[1]) == "live") {
            TheVideoCapturer.open(0);
         }
        else     TheVideoCapturer.open(argv[1]);
        // check video is open
        if (!TheVideoCapturer.isOpened()) {
            cerr << "Could not open video" << endl;
            return -1;
        }

        // read first image to get the dimensions
        TheVideoCapturer >> TheInputImage;

        //set specific parameters for this configuration
         MarkerDetector::Params params;
         //play with this paramteres if the detection does not work correctly
        params._borderDistThres=.01;//acept markers near the borders
        params._thresParam1=5;
        params._thresParam1_range=5;//search in wide range of values for param1
        params._cornerMethod=MarkerDetector::SUBPIX;//use subpixel corner refinement
        params._subpix_wsize= (15./2000.)*float(TheInputImage.cols) ;//search corner subpix in a 5x5 widow area
        TheMarkerDetector.setParams(params);//set the params above
         TheMarkerDetector.setDictionary(TheMarkerMapConfig.getDictionary());
        // Create gui and prepare the detector for an aruco chessboard
        cout<<"Press 'a'' to add current view to the pool of images used for calibration"<<endl;
        cout<<"Press 'c' to perform calibration"<<endl;
        cout<<"Press 's' to start/stop capture"<<endl;
        cv::namedWindow("in", 1);
        iThresParam1=TheMarkerDetector.getParams()._thresParam1;
        iThresParam2=TheMarkerDetector.getParams()._thresParam2;

        cv::createTrackbar("ThresParam1", "in", &iThresParam1, 13, cvTackBarEvents);
        cv::createTrackbar("ThresParam2", "in", &iThresParam2, 13, cvTackBarEvents);
        char key = 0,capturing=0;
         // capture until press ESC or until the end of the video
        do {
            TheVideoCapturer.retrieve(TheInputImage);//get image
            // detect and print
            vector<aruco::Marker> detected_markers=TheMarkerDetector.detect(TheInputImage);
            vector<int> markers_from_set=TheMarkerMapConfig.getIndices(detected_markers);
            // print markers from the board
            TheInputImage.copyTo(TheInputImageCopy);
            for(auto idx:markers_from_set) detected_markers[idx].draw(TheInputImageCopy, Scalar(0, 0, 255), 1);

            // show input with augmented information and  the thresholded image
            cv::imshow("in", TheInputImageCopy);
         //   cv::imshow("thres", TheMarkerDetector.getThresholdedImage());

             // write to video if required

            while( (key = cv::waitKey(10))==-1 && !capturing) ; // wait for key to be pressed
             if (key=='a'){
                vector<cv::Point2f> p2d;
                vector<cv::Point3f> p3d;

                getMarker2d_3d(p2d,p3d,detected_markers,TheMarkerMapConfig);
                if(p3d.size()>0){
                    calib_p2d.push_back(p2d);
                    calib_p3d.push_back(p3d);
                }
            }
            bool calibrate=false;
            //calibrate if requested, or if going to leave
            if (key=='c' || (key==27 && calib_p2d.size()>2) ) calibrate=true;
            if (calibrate){
                vector<cv::Mat> vr,vt;
                camp.CamSize=TheInputImage.size();
                cout<<calib_p2d.size()<<endl;
                double err=cv::calibrateCamera(calib_p3d,calib_p2d,TheInputImage.size(),camp.CameraMatrix,camp.Distorsion,vr,vt);
                cerr<<"repj error="<<err<<endl;
                camp.saveToFile(argv[2]);
            }
            //set waitTime in start/stop mode
            if (key=='s') { capturing=!capturing;}
         } while (key != 27 && TheVideoCapturer.grab());


    } catch (std::exception &ex)

    {
        cout << "Exception :" << ex.what() << endl;
    }
}
/************************************
 *
 *
 *
 *
 ************************************/

void cvTackBarEvents(int pos, void *) {
    (void)(pos);
    if (iThresParam1 < 3) iThresParam1 = 3;
    if (iThresParam1 % 2 != 1) iThresParam1++;
    if (iThresParam2 < 1) iThresParam2 = 1;


    MarkerDetector::Params p=    TheMarkerDetector.getParams();
    p._thresParam1=iThresParam1;
    p._thresParam2=iThresParam2;
    TheMarkerDetector.setParams(p);
    // detect and print
    vector<aruco::Marker> detected_markers=TheMarkerDetector.detect(TheInputImage);
    vector<int> markers_from_set=TheMarkerMapConfig.getIndices(detected_markers);
    TheInputImage.copyTo(TheInputImageCopy);
    for(auto idx:markers_from_set) detected_markers[idx].draw(TheInputImageCopy, Scalar(0, 0, 255), 1);

    cv::imshow("in", TheInputImageCopy);
    cv::waitKey(10);
}
