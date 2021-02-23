set(Wallet_Tests
    "Test_GetStealthAddress.cpp"
#    "Test_Pegins.cpp"
#    "Test_PegOut.cpp"
#    "Test_SendReceive.cpp"
)

list_prepend(Wallet_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Wallet_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)