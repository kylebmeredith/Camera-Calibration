# Install script for directory: /home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src

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
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/build/src/libaruco.a")
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "main")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/aruco" TYPE FILE FILES
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/markerlabeler.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/cvdrawingutils.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/ippe.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/posetracker.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/dictionary.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/checkrectcontour.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/ar_omp.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/levmarq.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/marker.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/markermap.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/cameraparameters.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/markerdetector.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/aruco.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/exports.h"
    "/home/kbmeredith/public_html/Summer_Research_2017/aruco-2.0.19/src/markerlabelers/dictionary_based.h"
    )
endif()

