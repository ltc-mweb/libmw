set(Serialization_Tests
    "Test_Deserializer.cpp"
    "Test_Serializer.cpp"
    "Test_Json.cpp"
)

list(TRANSFORM Serialization_Tests PREPEND ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Serialization_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)