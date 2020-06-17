set(TARGET_NAME crypto_deps)

set(CRYPTO_SOURCES
    src/aes.cpp
    src/bech32.cpp
    src/blake2b.cpp
    src/hmac_sha256.cpp
    src/hmac_sha512.cpp
    src/ripemd160.cpp
    src/sha256.cpp
    src/sha512.cpp
    src/scrypt/crypto_scrypt-ref.cpp
    src/scrypt/sha256.cpp
    src/ctaes/ctaes.c
)

add_library(${TARGET_NAME} ${CRYPTO_SOURCES})
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/crypto)
target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_SCRYPT_CONFIG_H)