# Install script for directory: /Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "main" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/build/libaruco.a")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libaruco.a" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libaruco.a")
    execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libaruco.a")
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "main" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/aruco" TYPE FILE FILES
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/ar_omp.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/aruco.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/cameraparameters.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/checkrectcontour.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/cvdrawingutils.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/dictionary.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/exports.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/ippe.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/levmarq.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/marker.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/markerdetector.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/markerlabeler.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/markermap.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/posetracker.h"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/src/markerlabelers/dictionary_based.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/Kyle/Google Drive/#Summer Research 2017/Code/aruco-2.0.19/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
