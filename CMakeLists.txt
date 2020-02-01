cmake_minimum_required(VERSION 3.8)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project(LIBMW-LTC)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

if (NOT MW_CORE_ROOT)
	set(MW_CORE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../libmw-core")
endif ()

include(${MW_CORE_ROOT}/cmake/SetOutputDir.cmake)
include(${MW_CORE_ROOT}/cmake/SetOptimizations.cmake)
include(${MW_CORE_ROOT}/cmake/SetCpp17.cmake)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(WITH_GFLAGS OFF)

include_directories(
	${MW_CORE_ROOT}/include
	${PROJECT_SOURCE_DIR}/include
)

# Dependencies
include(${MW_CORE_ROOT}/deps/dependencies.cmake)

add_definitions(-DUNICODE)
add_definitions(-DNOMINMAX)
add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)

option(GRINPP_TESTS "Build tests" true)
if(GRINPP_TESTS AND NOT LINUX)
	add_subdirectory(tests)
endif(GRINPP_TESTS AND NOT LINUX)


#set(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

#include(ExternalProject)
#ExternalProject_Add(libmw-core
#	PREFIX 		${PARENT_DIR}/libmw-core
#	INSTALL_DIR	${PARENT_DIR}
#	CMAKE_ARGS
#		-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
#		-DCMAKE_PROJECT_geGL_INCLUDE:PATH=${PARENT_DIR}/include.cmake
#		-DROOT_DIR:PATH=${PARENT_DIR}
#		-DBUILD_SHARED_LIBS=ON
#
#	#GIT_REPOSITORY	https://github.com/GrinPlusPlus/libmw-core.git
#	#GIT_TAG			master # working
#)