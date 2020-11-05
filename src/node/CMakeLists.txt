set(TARGET_NAME Node)

file(GLOB SOURCE_CODE
	"Node.cpp"
	"BlockBuilder.cpp"
	"CoinsViewCache.cpp"
	"CoinsViewDB.cpp"
	"CoinsViewFactory.cpp"
	"ICoinsView.cpp"
	"validation/BlockValidator.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

add_dependencies(${TARGET_NAME} MW::Common MW::Crypto MW::Database MW::File MW::MMR MW::Traits)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common MW::Crypto MW::Database MW::File MW::MMR MW::Traits)