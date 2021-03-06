macro(SET_WARNING_LEVEL LEVEL)
	if(MSVC)
		if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
			string(REGEX REPLACE "/W[0-4]" "/W${LEVEL}" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W${LEVEL}")
		 endif()
	endif(MSVC)
endmacro()