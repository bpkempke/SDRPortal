#
##CMake version and language
#cmake_minimum_required(VERSION 2.6)
#
##Project name and details
#project(SDRPortal)
#set(SDRPortal_VERSION_MAJOR 0)
#set(SDRPortal_VERSION_MINOR 1)
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)
#
##Compiler-specific setup
#if(CMAKE_COMPILER_IS_GNUCXX)
#	add_definitions(-Wall)
#	add_definitions(-Wextra)
#	add_definitions(-Wno-unused)
#	add_definitions(-Wsign-compare)
#	add_definitions(-fvisibility=hidden)
#	add_definitions(-std=c++0x)
#endif()
#
##Set up the packages which are necessary in order to build the project
#find_package(Threads)
#find_package(UHD)
#find_package(LibHackRF)
#find_package(LibRTLSDR)
#
#include_directories(
#	${CMAKE_SOURCE_DIR}/lib
#)
#
#add_subdirectory(src)
#
#
# DSPFlow CMakeLists.txt
#
# Ben Kempke
#
# Shared libraries should be compiled as follows:
#  gcc -shared -nostartfiles -o bar bar.c
#
# Also make sure to install libdb in order to compile...:
#  sudo apt-get install libdb-dev

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
