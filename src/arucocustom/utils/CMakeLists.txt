INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${GNULIBS_INCLUDE_DIR})
LINK_LIBRARIES(${PROJECT_NAME} ${ARUCO_REQUIRED_LIBRARIES} )

ADD_EXECUTABLE(aruco_print_dictionary aruco_print_dictionary.cpp)
ADD_EXECUTABLE(aruco_print_marker aruco_print_marker.cpp)
ADD_EXECUTABLE(aruco_simple aruco_simple.cpp)
ADD_EXECUTABLE(aruco_test aruco_test.cpp)
ADD_EXECUTABLE(aruco_tracker aruco_tracker.cpp)


#aruco_create_board
INSTALL(TARGETS aruco_tracker aruco_simple aruco_test aruco_print_marker aruco_print_dictionary       RUNTIME DESTINATION bin)

