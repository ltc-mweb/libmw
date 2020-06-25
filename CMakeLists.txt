cmake_minimum_required(VERSION 3.8)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")

project(LIBMW-LTC)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetOutputDir.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetOptimizations.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetCpp17.cmake)

set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL MinSizeRel Release)
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO RelWithDebInfo Release)
#set_property(GLOBAL PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
if(MSVC)
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif(MSVC)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(WITH_GFLAGS OFF)

include_directories(
	${PROJECT_SOURCE_DIR}/include
)

# Dependencies
include(${CMAKE_CURRENT_SOURCE_DIR}/deps/vcpkg_deps.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/deps/dependencies.cmake)

if (WIN32)
    add_definitions(-DMW_ENABLE_WSTRING -DUNICODE -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()

add_subdirectory(src)

option(MW_TESTS "Build tests" true)
if(MW_TESTS AND NOT LINUX)
	add_subdirectory(test)
endif(MW_TESTS AND NOT LINUX)