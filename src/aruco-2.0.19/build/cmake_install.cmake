# Install script for directory: /Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19

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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake" TYPE FILE FILES "/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/Findaruco.cmake")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/aruco-uninstalled.pc"
    "/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/aruco.pc"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/src/cmake_install.cmake")
  include("/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/utils/cmake_install.cmake")
  include("/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/utils_markermap/cmake_install.cmake")
  include("/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/utils_calibration/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/Users/Kyle/Google Drive/#Summer Research 2017/Code/CameraCalibration/src/aruco-2.0.19/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
