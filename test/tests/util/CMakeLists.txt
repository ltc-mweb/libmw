set(Util_Tests
    "Test_BitUtil.cpp"
    "Test_EndianUtil.cpp"
    "Test_FileUtil.cpp"
    "Test_HexUtil.cpp"
    "Test_StringUtil.cpp"
    "Test_ThreadUtil.cpp"
    "Test_TimeUtil.cpp"
)

list_prepend(Util_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Util_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)