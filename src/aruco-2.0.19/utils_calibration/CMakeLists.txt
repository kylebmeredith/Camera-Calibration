INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${GNULIBS_INCLUDE_DIR})
LINK_LIBRARIES(${PROJECT_NAME} ${ARUCO_REQUIRED_LIBRARIES} )

 

ADD_EXECUTABLE(aruco_calibration aruco_calibration.cpp )
ADD_EXECUTABLE(aruco_calibration_fromimages aruco_calibration_fromimages.cpp )

INSTALL(TARGETS      aruco_calibration aruco_calibration_fromimages RUNTIME DESTINATION bin)

