set(TARGET_NAME Common)

file(GLOB SOURCE_CODE
	"LoggerImpl.cpp"
	"ThreadManagerImpl.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_compile_definitions(${TARGET_NAME} PRIVATE MW_COMMON)
add_dependencies(${TARGET_NAME} fmt::fmt spdlog::spdlog_header_only)
target_link_libraries(${TARGET_NAME} PUBLIC fmt::fmt spdlog::spdlog_header_only)
target_include_directories(${TARGET_NAME} INTERFACE ${Boost_INCLUDE_DIRS})