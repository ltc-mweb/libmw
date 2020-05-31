set(TARGET_NAME Node)

file(GLOB SOURCE_CODE
	"Node.cpp"
	"validation/BlockValidator.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

add_dependencies(${TARGET_NAME} MW::Common)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common)