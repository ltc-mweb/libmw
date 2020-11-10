string(REGEX REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2")
	 endif()
endif (MSVC)

# Serves the same purpose as list(TRANSFORM ... PREPEND ...),
# which isn't available in 3.10.
function(list_prepend LIST_NAME PREFIX)
    set(NEW_LIST "")
    foreach(element ${${LIST_NAME}})
        list(APPEND NEW_LIST "${PREFIX}${element}")
    endforeach(element)
    set(${LIST_NAME} "${NEW_LIST}" PARENT_SCOPE)
endfunction()

add_subdirectory(framework)

set(test_sources "TestMain.cpp")
add_subdirectory(tests/common)
add_subdirectory(tests/consensus)
add_subdirectory(tests/crypto)
add_subdirectory(tests/db)
add_subdirectory(tests/mmr)
add_subdirectory(tests/models)
add_subdirectory(tests/node)
add_subdirectory(tests/serialization)
add_subdirectory(tests/util)
add_subdirectory(tests/wallet)

add_executable(Tests ${test_sources})
add_dependencies(Tests fmt::fmt MW_TEST::Framework MW::Node)
target_link_libraries(Tests MW_TEST::Framework MW::Node Catch2::Catch2 libmw)

include(CTest)
include(Catch)
catch_discover_tests(Tests)

add_custom_command(TARGET Tests
                POST_BUILD
                COMMAND ctest -C $<CONFIGURATION> --output-on-failure)