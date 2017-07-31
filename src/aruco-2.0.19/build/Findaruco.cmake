# ===================================================================================
#  aruco CMake configuration file
#
#             ** File generated automatically, do not modify **
#
#  Usage from an external project:
#    In your CMakeLists.txt, add these lines:
#
#    FIND_PACKAGE(aruco REQUIRED )
#    TARGET_LINK_LIBRARIES(MY_TARGET_NAME )
#
#    This file will define the following variables:
#      - aruco_LIBS          : The list of libraries to links against.
#      - aruco_LIB_DIR       : The directory where lib files are. Calling LINK_DIRECTORIES
#                                with this path is NOT needed.
#      - aruco_VERSION       : The  version of this PROJECT_NAME build. Example: "1.2.0"
#      - aruco_VERSION_MAJOR : Major version part of VERSION. Example: "1"
#      - aruco_VERSION_MINOR : Minor version part of VERSION. Example: "2"
#      - aruco_VERSION_PATCH : Patch version part of VERSION. Example: "0"
#
# ===================================================================================
INCLUDE_DIRECTORIES("/usr/local/include")
SET(aruco_INCLUDE_DIRS "/usr/local/include")

LINK_DIRECTORIES("/usr/local/lib")
SET(aruco_LIB_DIR "/usr/local/lib")

SET(aruco_LIBS opencv_xphoto;opencv_xobjdetect;opencv_ximgproc;opencv_tracking;opencv_text;opencv_surface_matching;opencv_structured_light;opencv_stereo;opencv_saliency;opencv_rgbd;opencv_reg;opencv_plot;opencv_optflow;opencv_line_descriptor;opencv_hdf;opencv_fuzzy;opencv_face;opencv_dpm;opencv_dnn;opencv_datasets;opencv_cvv;opencv_ccalib;opencv_bioinspired;opencv_bgsegm;opencv_aruco;opencv_videostab;opencv_videoio;opencv_video;opencv_superres;opencv_stitching;opencv_shape;opencv_photo;opencv_objdetect;opencv_ml;opencv_imgproc;opencv_imgcodecs;opencv_highgui;opencv_flann;opencv_features2d;opencv_core;opencv_calib3d aruco)

SET(aruco_FOUND 1)
SET(aruco_VERSION        2.0.19)
SET(aruco_VERSION_MAJOR  2)
SET(aruco_VERSION_MINOR  0)
SET(aruco_VERSION_PATCH  1)
