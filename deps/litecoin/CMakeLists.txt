set(TARGET_NAME litecoin)

file(GLOB SOURCE_CODE
    bech32.cpp
    hash.cpp
    uint256.cpp
    crypto/aes.cpp
    crypto/bech32.cpp
    crypto/blake2b.cpp
    crypto/hmac_sha256.cpp
    crypto/hmac_sha512.cpp
    crypto/ripemd160.cpp
    crypto/sha256.cpp
    crypto/sha512.cpp
    crypto/scrypt/crypto_scrypt-ref.cpp
    crypto/scrypt/sha256.cpp
    crypto/ctaes/ctaes.c
    support/cleanse.cpp
    support/lockedpool.cpp
    util/strencodings.cpp
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
#target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_CONFIG_H)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})