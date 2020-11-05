set(Node_Tests
    "Test_BlockBuilder.cpp"
    "Test_MineChain.cpp"
    "Test_Reorg.cpp"
    "validation/Test_BlockValidator.cpp"
)

list_prepend(Node_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Node_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)