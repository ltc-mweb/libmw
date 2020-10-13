set(Consensus_Tests
    "Test_Aggregation.cpp"
    "Test_BlockSumValidator.cpp"
    "Test_CutThrough.cpp"
)

list(TRANSFORM Consensus_Tests PREPEND ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Consensus_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)