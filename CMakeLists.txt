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
	${MW_CORE_ROOT}/include
	${PROJECT_SOURCE_DIR}/include
)

# Dependencies
include(${MW_CORE_ROOT}/deps/vcpkg_deps.cmake)
include(${MW_CORE_ROOT}/deps/dependencies.cmake)
include(${MW_CORE_ROOT}/src/core.cmake)

add_subdirectory(src)

option(GRINPP_TESTS "Build tests" true)
if(GRINPP_TESTS AND NOT LINUX)
	add_subdirectory(tests)
endif(GRINPP_TESTS AND NOT LINUX)