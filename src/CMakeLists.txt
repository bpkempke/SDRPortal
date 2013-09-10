#    SDRPortal - A generic web-based interface for SDRs
#    Copyright (C) 2013 Ben Kempke (bpkempke@umich.edu)
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

add_library(sdrportalShared SHARED
	portalCommandSocket.cc
	portalProfile.cc
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
	streamConverter.cc
	streamConversionHelper.cc
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(uhdd uhd_daemon.cc)
add_executable(shelld shell_daemon.cc)
add_executable(genc gen_conversions.cc)

add_custom_command(
	OUTPUT streamConversionHelper.h streamConversionHelper.cc
	COMMAND genc ${CMAKE_CURRENT_SOURCE_DIR}/types.txt streamConversionHelper.h streamConversionHelper.cc
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

