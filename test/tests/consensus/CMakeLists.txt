set(Consensus_Tests
    "Test_Aggregation.cpp"
    "Test_KernelSumValidator.cpp"
    "Test_Weight.cpp"
)

list_prepend(Consensus_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Consensus_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)