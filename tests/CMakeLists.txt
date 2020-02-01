include_directories(${PROJECT_SOURCE_DIR}/deps/Catch2)

string(REGEX REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2")
	 endif()
endif (MSVC)

add_subdirectory(models)

add_executable(Tests TestMain.cpp)
add_dependencies(Tests fmt)
target_link_libraries(Tests reproc++ fmt)