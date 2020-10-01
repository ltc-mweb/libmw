set(TARGET_NAME File)

file(GLOB SOURCE_CODE
	"*.cpp"
)

add_library(${TARGET_NAME} STATIC ${SOURCE_CODE})
add_library(MW::${TARGET_NAME} ALIAS ${TARGET_NAME})

add_dependencies(${TARGET_NAME} MW::Common MW::Traits)
target_link_libraries(${TARGET_NAME} PUBLIC MW::Common MW::Traits)

find_library(STDC++FS stdc++fs)
if (STDC++FS)
	target_link_libraries(${TARGET_NAME} PUBLIC ${STDC++FS})
endif()