INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${GNULIBS_INCLUDE_DIR})
LINK_LIBRARIES(${PROJECT_NAME} ${REQUIRED_LIBRARIES} )
set_source_files_properties(create_artag_hrm_dict.cpp PROPERTIES COMPILE_FLAGS -std=c++11)
set_source_files_properties(create_aruco_dict.cpp PROPERTIES COMPILE_FLAGS -std=c++11)

ADD_EXECUTABLE(create_artag_hrm_dict create_artag_hrm_dict.cpp)
ADD_EXECUTABLE(create_aruco_dict create_aruco_dict.cpp )
ADD_EXECUTABLE(mset2dict mset2dict.cpp highlyreliablemarkers.h highlyreliablemarkers.cpp)
ADD_EXECUTABLE(image2contourfile image2contourfile.cpp)
ADD_EXECUTABLE(extract_chilitag_dict extract_chilitag_dict.cpp)
ADD_EXECUTABLE(compute_dist_distance compute_dist_distance.cpp)

#ADD_EXECUTABLE(aruco_test_calibrationboard aruco_test_calibrationboard.cpp)

  
