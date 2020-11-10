cmake_minimum_required(VERSION 3.8)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")

project(LIBMW-LTC)

option(MW_TESTS "Build tests" true)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetWarningLevel4.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetOutputDir.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetOptimizations.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetCpp17.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetRuntimeLibrary.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetUsePthread.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/SetVisibilityHidden.cmake)

set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL MinSizeRel Release)
set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO RelWithDebInfo Release)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(WITH_GFLAGS OFF)

include_directories(
	${PROJECT_SOURCE_DIR}/include
)

# Dependencies
include(${CMAKE_CURRENT_SOURCE_DIR}/deps/vcpkg_deps.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/deps/dependencies.cmake)

if (WIN32)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN) # -DMW_ENABLE_WSTRING -DUNICODE 
endif()

add_subdirectory(src)

# Install includes
install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/libmw/
    DESTINATION include/libmw
)

if(MW_TESTS)
	add_subdirectory(test)
endif()