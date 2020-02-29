set(TARGET_NAME Models_Tests)

file(GLOB SOURCE_CODE
    "*.cpp"
	"block/*.cpp"
	"tx/*.cpp"
)

add_executable(${TARGET_NAME} ${SOURCE_CODE})
target_link_libraries(${TARGET_NAME} fmt Core::Common Core::Traits Core::Crypto)