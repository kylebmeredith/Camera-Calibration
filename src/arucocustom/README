ArUco: One library to rule them all
-------------------------------------------------------------------

ArUco is an easy-to-use C++ library for detection of squared planar markers based on OpenCv. It can detect markers from a variety of ditionaries ARUCO, APRILTAGS, ARTAG and ARTOOLKIT+.
It is a library, to rule them all ;)

CONTACT: Rafael Munoz-Salinas: rmsalinas@uco.es 

\section INTRODUCTION

The library allows to detect squared planar markers in images. A   marker is a squared element with a black border and a inner binary code inside. The set of valid codes is called dictionary. Several libraries have been proposed, and each one usually includes its own dictionary of valid markers. This version of Aruco allows to use the most famous dictionaries: ARUCO,APRILTAGS,ARTAG and ARTOOLKIT+.


The library allows to estimate the pose of a camera from one or many  markers.

In any case, we strongly recommend to use our dictionary ARUCO_MIP_36h12,  which in our opinion, is the one that has the best trade-off between size and robustness. See https://www.researchgate.net/publication/282426080_Generation_of_fiducial_marker_dictionaries_using_mixed_integer_linear_programming for more information on dictionary generation.

The library also allows to use marker maps. A marker map is a set of markers placed in the environment whose location w.r.t. each other does not change. The simplest marker map supported by the library is a set of markers printed in a piece of paper. Localization in  more robust since the misdetection of several markers of the map is not a problem as long as a minimum set of them are detected.

Additionally, marker maps can also extend to much larger environments. Imagine you place markers in a room and you want to locate using them. Now, aruco allows you to do so using the brother library aruco_markermapper.


For additional information to this README file, please visit our web page : http://www.uco.es/investiga/grupos/ava/node/26

\section What's new in 2.x and how to port

See Changelog for a more detailed description

		- Added the Dictionary concept which  allows to use the most famous fiducial marker approaches : ARUCO,APRILTAGS,ARTAG and ARTOOLKIT+. These are hardcoded so you only have to select the one you need.
		- Parallelization improved and some other optimizations added
		- Boards are now replaced by the concept of MarkerMap which is a more general concept.
		- Added classes for tracking the camera pose. 
		- Chessboard patterns are now allowed
		- Calibration tool included


The main difference with previous library is that you can select amongst a set of Dictionaries:   ARUCO_MIP_25h7,ARUCO_MIP_16h3, and ARUCO_MIP_36h12 which are hardcoded. Thus, you do not need external files describing the ditionary.

We recommend using ARUCO_MIP_36h12, however, for compatibilty with old users, the default dictionary of the library is ARUCO, which is the original dictionary in previuos versions.  However, we recommend using ARUCO_MIP_36h12 because it is more robust to errors.

The concept of Board (In 1.x version) is replaced by  MarkerMaps. 

Also important, from this moment, the axis point in the same direction as given by opencv, i.e., Z axis is perperdicular to the marker. 


With respect to performance, we have improved in several aspects. First, all markers are now analized in parallel (if omp is enabled/found at compile time). Second, we have added a pyramid method for marker detection. Once detected rectangles in the first level, we create the canonical image in the pyramid level that achieves the best trade-off between quality and size. So, the call to wrap function is now more constant. So, you'll not notice decrease in performance as you approach a marker and it becomes bigger.

\section COMPILING

Use cmake.

In linux: (cd path;mkdir build;cd build cmake ..;make -j4;)

In windows, use cmake-gui, or QtCreator if you have not a lot of experience.

Set the -DOPENCV_DIR variable pointing to the directory where it is.

To consider:
	-DUSE_OWN_EIGEN3=ON/OFF  : We need the eigen3 library. So we have a copy of it in 3drparty. However, if you prefer to use yours, set this parameter to OFF
	-USE_DOUBLE_PRECISION_PNP=ON/OFF  The classes MarkerPoseTracker and   MarkerMapPoseTracker uses an optimization technique to estiamte the pose. By default we use double, but you can disable this option and float will be used instead.

\section TESTING

	Download the aruco test data from sourceforge and run the examples in the utils_xxx folders
	
\section LIBRARY DESCRIPTION:

The ArUco library contents are divided in several  directories. 

src/: contains the library
3rdparty/: contains the external code required. Currently only eigen3 is in it. If you prefer using your own eigen3 see in COMPILING section
utils/: utils for detection of individual markers.
utils_markermap: utils for using markermaps
utils_calibration: camera calibration utils. We provide a chessboard-like calibration board. You do need to see complete to do calibration so it is much better!
utils_gl: examples on how to use the library with OpenGL



The library main classes are:

   - aruco::Marker: which represent a marker detected in the image
   - aruco::MarkerDetector: that is in charge of deteting the markers in a image Detection is done by simple calling the member funcion ArMarkerDetector::detect(). You can select the dictionary you want to use calling 
		ArMarkerDetector::setDictionary(). (see programs in utils)
   - aruco::MarkerPoseTracker: tracks the pose of the camera wrt to a marker. Instead of recalculating the pose everytime from scratch, the tracker takes advantage of knowing the previous location to refine the current location. This is specially useful when the ambiguity problem arises. The ambiguity problem refers to the fact that some times there are more than one valid poses for the given view. In that cases, the pose returned by the estimator is any of them. However, using the tracker you can avoid this problem in many cases. See utils/aruco_tracker.cpp for an example.

   - aruco::MarkerMap: represents a set of markers whose location is known wrt to each other. (see programs in utils_markermap)
   - aruco::MarkerMapPoseTracker: is the class employed to estimate the pose of the camera wrt the set of markers.
   - aruco::CvDrawingUtils: a class with some routines for drawing in opencv images


\subsection Marker Maps

Marker maps exteds the concept of Board in the previous version of the library

Image you want to locate a robot in a laboratory. Then, you can place markers around the environment, record a video with the camera, and automatically obtain the location of the markers (this is call the marker map).
With the marker map, you can now locate the robot by looking a any of the makers.


You can create simple marker maps with the aruco libary, such as boards. A board is a grid of markers printed on a piece of paper. Since they are all in the same piece of paper, their location wrt each other are known in advance. 

However, for the case of the robot previouly explained, it is not known 3D the locations of the markers. So the  library marker_mapper allows to determine their location from a video sequence, and then, use the result in tacking tasks using aruco library. See the example utils_markermap/aruco_test_markermap.cpp to know how to do the job. The program also generates a pcd file showing the 3d location of the markers and of the camera along its trayectory. Additionally, the program can also output the 3d location to a log file.



\subsection APPLICATIONS

- The library comes with several applications that will help you to learn how to use the library:
 -# utils/aruco_print_marker: which creates marker and saves it in a jpg file you can print.
 -# utils/aruco_print_dictionary: saves to a dictionary all the markers of the dictionary indicated(ARUCO,APRILTAGS,ARTOOLKIT+,etc).
 -# utils/aruco_print_dictionary: saves to a dictionary all the markers of the dictionary indicated(ARUCO,APRILTAGS,ARTOOLKIT+,etc).
 -# utils/aruco_simple : simple test aplication that detects the markers in an image
 -# utils/aruco_test: this is the main application for detection. It reads images either from the camera of from a video and detect markers. Additionally, if you provide the intrinsics of the camera(obtained by OpenCv calibration) and the size of the marker in meters, the library calculates the marker intrinsics so that you can easily create your AR applications.
 -# utils/aruco_tracker: example showing how to use the tracker. 


 -# utils_markermap/aruco_create_markermap: creation of simple marker maps (the old boards). It creates a grid of markers that can be printed in a piece of paper.
	The result of this program are two files (.png and .yml) The png file is an image of the marker you can print. The .yml file is the configuration file that you'll need to pass to the rest of the programs, so they know how the map is.  The .yml contains the location of the markers in pixels. Since we do not know in advance how large the printed marker will be, we use pixels here. However, in order to obtain the camera, w need to know the real size of the marker. For this pourpose, you can either use the program  utils_markermap/aruco_markermap_pix2meters, that creates a new .yml, with the map information in meters. Alternatively, all the test programs allows you to indicate the markersize in the command line. 
 
 -# utils_markermap/aruco_markermap_pix2meters converts a markermap configuration file from pixels to meters
 -# utils_markermap/aruco_simple_markermap : simple example showing how to determine the camera pose using the marker maps
 -# utils_markermap/aruco_test_markermap : a bit more elaborated example showing how to determine the camera pose using the marker maps
 
  
 -# utils_calibration/aruco_calibration : a program to calibrate a camera using a chessboard comprised by aruco markers. In this directory you can find the printable calibration board aruco_calibration_board_a4.pdf. It is a marker map, whose definition is in aruco_calibration_board_a4.yml. However, it is hardcoded in the program so you do not even need to pass it as a parameter.
 -# utils_calibration/aruco_calibration_fromimages the same as above, but from images saved in a file

 -# utils_gl/aruco_test_gl simple example showing how to combine aruco with OpenGL
 
 NOTE ON OPENGL: The library supports  the integration with OpenGL. In order to compile with support for OpenGL, you just have  installed in your system the develop packages for GL and glut (or freeglut).
 


\section TIPS

You an increase speed by reducing the image size. If you do so, call CameraParameters::resize so that they adapt properly.

\section Robust Detection

\subsection Threshold parameters
By default, the MarkerDetector process the image to detect rectangles. Then, rectangles are examined to see if they are markers or not.

Due to illumination issues, rectangles might not be detected. The process involved in detecting rectangles is regulated by a set of variables MarkerDetector::Params you can work with to
improve the detection process, at the expenses of reducing the speed in some cases.

First, the image is binarized using a method that has two parameters MarkerDetector::Params::_thresParam1 and MarkerDetector::Params::_thresParam2.
The first parameter is the blockSize, i.e., the size of the sliding window employed for binarization. The second is the binarization threshold.

The library allows you to specify a range of values for MarkerDetector::Params::_thresParam1 using the variable MarkerDetector::Params::_thresParam1_range.
By default it is set to 0, but if you find dificulties you can increase its value. It will search in a range [_thresParam1 -_thresParam1_range , _thresParam1 +_thresParam1_range].

To set or get the parameters, use the member functions MarkerDetector::getParams() and MarkerDetector::setParams()


\section Marker Size

Another important parameter that conditions the detection is the size of the analyzed squares. After detecting squares, we reject these not falling in a certain size limit. This is  specified by the variables MarkerDetector::Params::_minSize and MarkerDetector::Params::_maxSize.

Both variables are in the range [0,1], where 1 means the maximum possible size a marker can have in the given image.  As you see, this is a normalized value.


This prevents noise (too small or to big squares to be analyzed). However, in some cases it might no make sense. If you experience difficulties detecting the markers, try setting min to 0 and  max to 1.






