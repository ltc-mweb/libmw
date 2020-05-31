set(Node_Tests
    "validation/Test_BlockValidator.cpp"
)

list(TRANSFORM Node_Tests PREPEND ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Node_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)