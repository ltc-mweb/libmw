include_directories(${MW_CORE_ROOT}/deps/Catch2)

string(REGEX REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if (MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2")
	 endif()
endif (MSVC)

set(Models_Tests
	"src/models/block/Test_Block.cpp"
	"src/models/block/Test_Header.cpp"
	"src/models/tx/Test_Kernel.cpp"
)

set(Chain_Tests
	"src/chain/Test_ChainState.cpp"
)

set(Node_Tests
    "src/node/validation/Test_BlockValidator.cpp"
)

add_executable(Tests TestMain.cpp ${Models_Tests} ${Chain_Tests} ${Node_Tests})
add_dependencies(Tests fmt::fmt)
target_link_libraries(Tests reproc++ fmt::fmt Core::Common Core::Crypto LTC::Node)
target_include_directories(Tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)