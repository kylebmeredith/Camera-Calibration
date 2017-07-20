INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src ${GNULIBS_INCLUDE_DIR})
LINK_LIBRARIES(${PROJECT_NAME} ${REQUIRED_LIBRARIES} )

IF(GL_FOUND AND BUILD_GLSAMPLES)
  ADD_EXECUTABLE(aruco_test_gl aruco_test_gl.cpp)
  TARGET_LINK_LIBRARIES(aruco_test_gl ${OPENGL_LIBS})

ADD_EXECUTABLE(aruco_test_markermap_gl aruco_test_markermap_gl.cpp)
 TARGET_LINK_LIBRARIES(aruco_test_markermap_gl ${OPENGL_LIBS})

#ADD_EXECUTABLE(aruco_test_board_gl_mask aruco_test_board_gl_mask.cpp)
#TARGET_LINK_LIBRARIES(aruco_test_board_gl_mask ${OPENGL_LIBS})

 INSTALL(TARGETS aruco_test_gl aruco_test_markermap_gl   RUNTIME DESTINATION bin)
ENDIF()
