#TODO: Add GPL stuff...
add_library(sdrportalShared SHARED
	portalCommandSocket.cc
	shellPortal.cc
	base64.cc
	generic.cc
	genericSDRInterface.cc
	genericSocketInterface.cc
	hackRFInterface.cc
	hierarchicalDataflowBlock.cc
	portalDataSocket.cc
	rtlInterface.cc
	sha1.cc
	uhdInterface.cc
)

add_executable(uhdd uhd_daemon.cc)
add_executable(shelld shell_daemon.cc)
add_executable(genc gen_conversions.cc)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/streamConversionHelper.h ${CMAKE_CURRENT_BINARY_DIR}/streamConversionHelper.cc
	COMMAND genc types.txt ${CMAKE_CURRENT_BINARY_DIR}/streamConversionHelper.h ${CMAKE_CURRENT_BINARY_DIR}/streamConversionHelper.cc
	DEPENDS genc
)

target_link_libraries(sdrportalShared	
	${LIBHACKRF_LIBRARIES}
	${LIBRTLSDR_LIBRARIES}
	${UHD_LIBRARIES}
)

target_link_libraries(uhdd 
	sdrportalShared
	${CMAKE_THREAD_LIBS_INIT}
)
target_link_libraries(shelld
	sdrportalShared
	${CMAKE_THREAD_LIBS_INIT}
)

