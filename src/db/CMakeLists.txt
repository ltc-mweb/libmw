set(TARGET_NAME Database)

file(GLOB SOURCE_CODE
	"*.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_compile_definitions(${TARGET_NAME} PRIVATE MW_DATABASE)

add_dependencies(${TARGET_NAME} leveldb::leveldb MW::Common MW::Traits MW::Crypto)
target_link_libraries(${TARGET_NAME} PUBLIC leveldb::leveldb MW::Common MW::Traits MW::Crypto)