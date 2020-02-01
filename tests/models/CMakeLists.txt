set(TARGET_NAME Models_Tests)

file(GLOB SOURCE_CODE
    "*.cpp"
	"block/*.cpp"
)

add_executable(${TARGET_NAME} ${SOURCE_CODE})
target_link_libraries(${TARGET_NAME} fmt)