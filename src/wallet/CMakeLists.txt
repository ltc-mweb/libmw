set(TARGET_NAME Wallet)

add_library(${TARGET_NAME} STATIC
	"Transact.cpp"
	"Wallet.cpp"
)

add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common MW::Crypto MW::Models)