
if(NOT BUILD_DIR)
    set(BUILD_DIR build)
endif()

add_subdirectory(common)
add_subdirectory(traits)
add_subdirectory(file)
add_subdirectory(crypto)
add_subdirectory(db)
add_subdirectory(mmr)
add_subdirectory(node)