set(TARGET_NAME Database)

file(GLOB SOURCE_CODE
	"CoinDB.cpp"
	"LeafDB.cpp"
	"MMRInfoDB.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_compile_definitions(${TARGET_NAME} PRIVATE MW_DATABASE)

add_dependencies(${TARGET_NAME} MW::Common MW::Traits MW::Crypto)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common MW::Traits MW::Crypto)