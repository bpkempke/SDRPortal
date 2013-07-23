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

cmake_minimum_required(VERSION 2.6)
project(SDRPortal)

#Version 0.1
set(SDRPortal_VERSION_MAJOR 0)
set(SDRPortal_VERSION_MINOR 1)

#Set the paths to find the correct header files
include_directories(
	${CMAKE_SOURCE_DIR}/lib
)

#And the library which provides dynamic linking (will be different for NaCl though...)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

#Find the threading library 'package'
find_package(Threads)
find_package(UHD)
find_package(LibHackRF)
find_package(LibRTLSDR)
find_package(Portaudio)

#Add in debugging symbols for now... TODO: Get rid of debugging symbols
set(CMAKE_CXX_FLAGS "-g -Wall -std=c++0x")

add_subdirectory(src)
