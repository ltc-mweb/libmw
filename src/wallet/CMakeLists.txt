set(TARGET_NAME Wallet)

file(GLOB SOURCE_CODE
	"PegIn.cpp"
	"Wallet.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_link_libraries(${TARGET_NAME} PUBLIC MW::Common MW::Crypto)