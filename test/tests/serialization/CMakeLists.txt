set(Serialization_Tests
    "Test_Deserializer.cpp"
    "Test_Serializer.cpp"
)

list_prepend(Serialization_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Serialization_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)