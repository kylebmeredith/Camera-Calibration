INCLUDE_DIRECTORIES(.)


FILE(GLOB hdrs "*.h*")
FILE(GLOB srcs "*.c*")




SET(detectors_src   markerlabelers/dictionary_based.cpp   )# markerlabelers/svmmarkers.cpp )
SET(detectors_hdr   markerlabelers/dictionary_based.h     )# markerlabelers/svmmarkers.h)

SET(hdrs ${hdrs} ${detectors_hdr})
SET(srcs ${srcs} ${detectors_src})

SET(THIS_LIBNAME aruco)
ADD_LIBRARY(${THIS_LIBNAME}  ${srcs} ${hdrs})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} )
MESSAGE(STATUS "OpenCV_LIB_DIR=${OpenCV_LIB_DIR}")
SET_TARGET_PROPERTIES(${THIS_LIBNAME} PROPERTIES 				# create *nix style library versions + symbolic links
        DEFINE_SYMBOL ARUCO_DSO_EXPORTS
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_SOVERSION}
	CLEAN_DIRECT_OUTPUT 1 					# allow creating static and shared libs without conflicts
        OUTPUT_NAME "${THIS_LIBNAME}${PROJECT_DLLVERSION}" 	# avoid conflicts between library and binary target names
)

TARGET_LINK_LIBRARIES(${THIS_LIBNAME} ${ARUCO_REQUIRED_LIBRARIES})

INSTALL(TARGETS ${THIS_LIBNAME}
	RUNTIME DESTINATION bin COMPONENT main			# Install the dll file in bin directory
	LIBRARY DESTINATION lib PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE COMPONENT main
	ARCHIVE DESTINATION lib COMPONENT main)			# Install the dll.a file in lib directory

INSTALL(FILES ${hdrs}
        DESTINATION include/${THIS_LIBNAME}
	COMPONENT main)

