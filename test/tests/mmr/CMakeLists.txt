set(MMR_Tests
    "Test_FileBackend.cpp"
    "Test_Index.cpp"
    "Test_LeafIndex.cpp"
	"Test_LeafSet.cpp"
	"Test_LeafSetCache.cpp"
    "Test_MMR.cpp"
)

list(TRANSFORM MMR_Tests PREPEND ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${MMR_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)