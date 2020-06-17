set(Chain_Tests
	"Test_ChainState.cpp"
)

list(TRANSFORM Chain_Tests PREPEND ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Chain_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)