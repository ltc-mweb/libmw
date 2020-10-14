set(Common_Tests
    "Test_Scheduler.cpp"
)

list_prepend(Common_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Common_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)