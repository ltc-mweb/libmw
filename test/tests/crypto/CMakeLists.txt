set(Crypto_Tests
    "Test_AddCommitments.cpp"
    "Test_AggSig.cpp"
    "Test_RangeProofs.cpp"
)

list_prepend(Crypto_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Crypto_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)