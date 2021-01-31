set(TARGET_NAME Models)

file(GLOB SOURCE_CODE
	"block/Block.cpp"
	"tx/Kernel.cpp"
	"tx/Output.cpp"
	"tx/OwnerData.cpp"
	"tx/Transaction.cpp"
	"tx/TxBody.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

add_dependencies(${TARGET_NAME} MW::Common MW::Traits)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common MW::Traits)