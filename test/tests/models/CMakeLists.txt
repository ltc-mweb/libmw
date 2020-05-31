set(Models_Tests
	"block/Test_Block.cpp"
	"block/Test_Header.cpp"
	"crypto/Test_BigInteger.cpp"
	"tx/Test_Kernel.cpp"
)

list(TRANSFORM Models_Tests PREPEND ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Models_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)