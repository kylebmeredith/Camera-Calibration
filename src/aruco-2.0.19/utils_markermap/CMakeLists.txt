INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${GNULIBS_INCLUDE_DIR})
LINK_LIBRARIES(${PROJECT_NAME} ${ARUCO_REQUIRED_LIBRARIES} )

 


ADD_EXECUTABLE(aruco_create_markermap aruco_create_markermap.cpp)


ADD_EXECUTABLE(aruco_simple_markermap aruco_simple_markermap.cpp)
ADD_EXECUTABLE(aruco_test_markermap aruco_test_markermap.cpp pcdwriter.cpp)
ADD_EXECUTABLE(aruco_markermap_pix2meters aruco_markermap_pix2meters.cpp)
 

INSTALL(TARGETS     aruco_markermap_pix2meters     aruco_simple_markermap aruco_test_markermap    RUNTIME DESTINATION bin)

