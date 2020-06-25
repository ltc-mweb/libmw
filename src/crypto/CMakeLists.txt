set(TARGET_NAME Crypto)

file(GLOB SOURCE_CODE
	"Bulletproofs.cpp"
	"ConversionUtil.cpp"
	"Crypto.cpp"
	"MuSig.cpp"
	"Pedersen.cpp"
	"PublicKeys.cpp"
	"Schnorr.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_compile_definitions(${TARGET_NAME} PRIVATE MW_CRYPTO)

add_dependencies(${TARGET_NAME} crypto_deps secp256k1-zkp fmt::fmt MW::Common MW::Traits)
target_link_libraries(${TARGET_NAME} PUBLIC crypto_deps secp256k1-zkp fmt::fmt MW::Common MW::Traits)