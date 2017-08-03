

# Camera Calibration
A comprehensive C++ camera calibration program that performs both intrinsic and stereo
calibration and supports a non planar calibration rig.

## Overview
This program performs two types of camera calibration, which are essential steps in many
applications related to computer vision. The first type is intrinsic camera calibration,
which determines the intrinsic camera matrix and distortion coefficients. It uses a set of images
of a calibration pattern with known geometry, such as a chessboard. These resulting intrinsics
can be used to correct an image's radial distortion. They are also inputted as a model for the camera
in the next type of calibration: extrinsic stereo calibration. Stereo calibration determines
the spatial relationship between the two viewpoints of a stereo camera setup, calculating the
rotation and translation transformations. The program uses these extrinsic parameters to produce
rectified image pairs, which are required for the important process of stereo matching.
In a perfectly rectified pair, each point on one image is on the same horizontal line as its
corresponding point in the other image.

The program integrates OpenCV functions with Rafael Munoz-Salinas' [ArUco library](https://sourceforge.net/projects/aruco/), adapted to support a novel 3D ArUco calibration rig.
Calibration with a non planar rig
requires precalculated camera intrinsics, so the program can input intrinsics from a file
(this could be easily adapted to support any sort of non planar rig). The program is controlled
by a comprehensive settings class, which is inputted as a YAML or XML file. These settings support a
host of UI and output features, such as a live camera preview mode, the ability to save detected,
undistorted, and rectified images, and the ability to display the 3D coordinates of an ArUco pattern.

[img](utils/readme/arucobox.png)

This program was developed for a Middlebury College undergraduate summer research project,
led by professor Daniel Scharstein. Camera calibration will be one component of
a pipeline designed to create datasets for 3D reconstruction on mobile devices.
These datasets will become the next entry in the well known Middlebury Computer Vision Benchmark http://vision.middlebury.edu/.

## Installation
### Dependencies
This program requires [OpenCV](http://opencv.org/releases.html) and the
[ArUco library](https://sourceforge.net/projects/aruco/). In OpenCV 3, the ArUco library
is a built in module. This program was developed in OpenCV 2.4.13, and it uses custom ArUco functions
based on the independent and distinct ArUco library linked above. The built in module defines
different classes and functions, so the two are not easily interchangeable.
This program can still be used in OpenCV 3 (there is a branch with the required changes), but that version
does not (yet) use the built in module.

### Instructions
The ArUco library is included in the [source directory](src/aruco-2.0.19). The included copy
is slightly modified from the vanilla sourceforge download. The [CMake file](src/aruco-2.0.19/CMakeLists.txt)
has been modified to build a static library and skip the utilities and tests (lines 43-46). The unrequired
source code and directories have also been removed. To build the library, navigate to its home directory
and execute the following:
```
mkdir build
cd build
cmake ..
make
```
This process might produce warnings about comparison or OpenMP, but these don't matter
as long as the static library is built. After this library is built, return to the Camera Calibration
home directory and execute `make`. This will create a build folder with the executable
calibrateWithSettings, which runs calibration from an inputted settings file.

## Functionality
Image order for stereo calibration for two views A and B: A B A B A B

For

Saving images:
NOTE: the path must already be created to be written to.
  detected
  undistorted
  rectified

  Because of this intrinsic input functionality, this code could be adapted to support any sort of non planar calibration rig
