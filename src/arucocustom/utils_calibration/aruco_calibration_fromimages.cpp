// /*****************************
// Copyright 2011 Rafael Muñoz Salinas. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice, this list of
//       conditions and the following disclaimer.
//
//    2. Redistributions in binary form must reproduce the above copyright notice, this list
//       of conditions and the following disclaimer in the documentation and/or other materials
//       provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY Rafael Muñoz Salinas ''AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Rafael Muñoz Salinas OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those of the
// authors and should not be interpreted as representing official policies, either expressed
// or implied, of Rafael Muñoz Salinas.
// ********************************/
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>
// #include <opencv2/calib3d/calib3d.hpp>
// #include "aruco.h"
// #include "aruco_calibration_board_a4.h"
// using namespace cv;
// using namespace aruco;
//
// Mat TheInputImage, TheInputImageCopy;
// CameraParameters TheCameraParameters;
// MarkerMap TheMarkerMapConfig;
// MarkerDetector TheMarkerDetector;
// string TheOutFile;
// string TheMarkerMapConfigFile;
// float TheMarkerSize=-1;
// bool printCoords=0;
// string TheOutCameraParams;
//
// //given the set of markers detected, the function determines the 2d-3d correspondance
// void getMarkerObjectPoints(vector<cv::Point2f> &imagePoints, vector<cv::Point3f> &objectPoints, const vector< Marker >&markers_detected,const MarkerMap &bc){
//     imagePoints.clear();
//     objectPoints.clear();
//     //for each detected marker
//     for(size_t i=0;i<markers_detected.size();i++){
//         //find it in the bc
//         int fidx=-1;
//         for(size_t j=0;j<bc.size() &&fidx==-1;j++)
//             if (bc[j].id==markers_detected[i].id ) fidx=j;
//         if (fidx!=-1){
//             for(int j=0;j<4;j++){
//                 imagePoints.push_back(markers_detected[i][j]);
//                 objectPoints.push_back(bc[fidx][j]);
//             }
//         }
//     }
//     cout<<"points added"<<endl;
// }
// vector < vector<cv::Point2f>  > calib_imagePoints;
// vector < vector<cv::Point3f>  > calib_objectPoints;
// aruco::CameraParameters camp;//camera parameters estimated
//
// vector<cv::Point3f> getIntPoints(vector<cv::Point3f> &xy){
//     vector<cv::Point3f> adapted;
//     for(auto p:xy) adapted.push_back(Point3f(((p.x+1125)/250), ((p.y+1125)/250), 0));
//     return adapted;
//     }
//
// vector<cv::Point3f> getIntPoints(vector<cv::Point3f> &xy, vector<cv::Point3f> &zy, vector<cv::Point3f> &xz){
//     vector<cv::Point3f> boxPoints;
//     // GENERALIZE THIS
//     for(auto p:xy) boxPoints.push_back(Point3f((p.x+1125)/250, (p.y+1125)/250, 0));
//     for(auto p:zy) boxPoints.push_back(Point3f(0, (p.y+1125)/250, (-p.x+1125)/250));
//     for(auto p:xz) boxPoints.push_back(Point3f((p.x+1125)/250, 0, (-p.y+1125)/250));
//     return boxPoints;
//     }
//
// int parseInput(int argc,char **argv){
//     TheOutFile=argv[1];
//     int lastOption=2;
//     for(int i=2;i<argc;i++){
//         if ( string(argv[i])=="-size" ){
//             TheMarkerSize=atof(argv[i+1]);
//             lastOption=i+2;
//         }
//         if ( string(argv[i])=="-m" ){
//             TheMarkerMapConfigFile=argv[i+1];
//             lastOption=i+2;
//         }
//         if ( string(argv[i])=="-c" ){
//             printCoords=argv[i+1];
//             lastOption=i+2;
//         }
//     }
//     return lastOption;
// }
// /************************************
//  *
//  *
//  *
//  *
//  ************************************/
int main(int argc, char **argv) {
//     try {
//
//         if (argc<4){
//             cerr << "Usage:   out_camera_calibration.yml [options] image1 image2  ... " << endl;
//             cerr<<"options:"<<endl;
//             cerr<<"-size maker_size : Size of the markers in mm. "<<endl;
//             cerr<<"-m markersetconfig.yml : By default, the one in utils is assumed. Otherwise, set the file here "<<endl;
//             cerr<<"-c printCoords (0 = print IDs, 1 = print coordinates)";
//             return -1;
//         }
//         // parse arguments
//         int firstImageIdx=parseInput(argc,argv);
//         //load board info
//         if ( TheMarkerMapConfigFile.empty()){
//             stringstream sstr;sstr.write((char*)default_a4_board,default_a4_board_size);
//             TheMarkerMapConfig.fromStream(sstr);
//         }else
//             TheMarkerMapConfig.readFromFile(TheMarkerMapConfigFile);
//         //is in meters
//         if (!TheMarkerMapConfig.isExpressedInMeters() ){
//             if (TheMarkerSize==-1){
//                 cerr<<"Need to specify the length of the board with -size"<<endl;
//                 return -1;
//             }
//             else TheMarkerMapConfig=TheMarkerMapConfig.convertToMeters(TheMarkerSize );
//         }
//         vector<string> images;
//         for(int i=firstImageIdx;i<argc;i++)images.push_back(argv[i]);
//         TheInputImage=cv::imread(images[0]);
//
//         //set specific parameters for this configuration
//         MarkerDetector::Params params;
//         params._borderDistThres=.01;//acept markers near the borders
//         params._maxSize=0.9;
//         params._thresParam1=5;
//         params._thresParam1_range=10;//search in wide range of values for param1
//         params._cornerMethod=MarkerDetector::SUBPIX;//use subpixel corner refinement
//         params._subpix_wsize= (10./2000.)*float(TheInputImage.cols) ;//search corner subpix in a window area
//         cout<<params._subpix_wsize<<" "<<float(TheInputImage.cols)<<endl;
//         TheMarkerDetector.setParams(params);//set the params above
//         TheMarkerDetector.setDictionary(TheMarkerMapConfig.getDictionary());
//
//
//          int currImage=0;
//         cv::Size imageSize(-1,-1);
//         // capture until press ESC or until the end of the video
//         do {
//             cout<<"reading "<<images[currImage]<<endl;
//             TheInputImage=cv::imread(images[currImage]);
//             if (!TheInputImage.empty()){
//                 if (imageSize!=cv::Size(-1,-1) &&  imageSize!=TheInputImage.size()){
//                     cerr<<"Input image sizes must be equal"<<endl;exit(0);
//                 }
//                 imageSize=TheInputImage.size();
//
//                 // detect and print
//                 vector<aruco::Marker> detected_markers=TheMarkerDetector.detect(TheInputImage);
//                 vector<int> markers_from_set=TheMarkerMapConfig.getIndices(detected_markers);
//
//                 vector<cv::Point2f> imagePoints;
//                 vector<cv::Point3f> objectPoints;
//
//                 getMarker2d_3d(imagePoints,objectPoints,detected_markers,TheMarkerMapConfig);
//                 if(objectPoints.size()>0){
//                     calib_imagePoints.push_back(imagePoints);
//                     calib_objectPoints.push_back(objectPoints);
//                 }
//                 cout << imagePoints;
//                 cout << objectPoints;
//                 vector<cv::Point3f> adapted = getIntPoints(objectPoints);
//                 //for(auto p:adapted) cout << p;
//
//                 TheInputImage.copyTo(TheInputImageCopy);
//                 // draw the markers onto the image
//                 // for final two paramters:
//                 //    false, true prints coordinates
//                 //    true, false prints id
//                 for(auto idx:markers_from_set) detected_markers[idx].draw(TheInputImageCopy, Scalar(0, 0, 255), std::max(float(1.f),1.5f*float(TheInputImageCopy.cols)/1000.f), adapted[idx*4], !printCoords, printCoords);
//
//                 if (TheInputImageCopy.cols>1280 )
//                     cv::resize(TheInputImageCopy,TheInputImage,cv::Size(1280, 1280.*float(TheInputImageCopy.rows)/float(TheInputImageCopy.cols)));
//                 else TheInputImageCopy.copyTo(TheInputImage);
//
//                 // show input with augmented information and  the thresholded image
//                 cv::imshow("in", TheInputImage);
//                 // write to video if required
//                 cv::waitKey(0); // wait for key to be pressed
//             }
//             else cerr<<"Could not read image "<<images[currImage]<<endl;
//         } while (++currImage<int(images.size()));
//
//         cout<<"Starting calibration"<<endl;
//         vector<cv::Mat> vr,vt;
//         camp.CamSize=imageSize;
//         cout<<calib_imagePoints.size()<<endl;
//         double err=cv::calibrateCamera(calib_objectPoints,calib_imagePoints,imageSize,camp.CameraMatrix,camp.Distorsion,vr,vt);
//         cerr<<"repj error="<<err<<endl;
//         camp.saveToFile(TheOutFile);
//         cerr<<"File saved in :"<<TheOutFile<<endl;
//
//     } catch (std::exception &ex)
//
//     {
//         cout << "Exception :" << ex.what() << endl;
//     }
}
