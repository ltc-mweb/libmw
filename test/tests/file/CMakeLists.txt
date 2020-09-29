set(File_Tests
 #   "Test_BitmapFile.cpp"
)

list(TRANSFORM File_Tests PREPEND ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${File_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)