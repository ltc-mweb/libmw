set(TARGET_NAME Framework)

file(GLOB SOURCE_CODE
	"src/TestFramework.cpp"
	"src/TxBuilder.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW_TEST::${TARGET_NAME} ALIAS ${TARGET_NAME})

add_dependencies(${TARGET_NAME} MW::Node)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Node)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)