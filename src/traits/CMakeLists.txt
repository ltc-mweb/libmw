set(TARGET_NAME Traits)

file(GLOB SOURCE_CODE
	"Serializable.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

add_dependencies(${TARGET_NAME} MW::Common)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common)