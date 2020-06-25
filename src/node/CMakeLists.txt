set(TARGET_NAME Node)

file(GLOB SOURCE_CODE
	"Node.cpp"
	"validation/BlockValidator.cpp"
)

add_library(${TARGET_NAME} SHARED ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_compile_definitions(${TARGET_NAME} PRIVATE BUILDING_NODE)
add_dependencies(${TARGET_NAME} MW::Common MW::Crypto MW::Database MW::File MW::MMR MW::Traits)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common MW::Crypto MW::Database MW::File MW::MMR MW::Traits)