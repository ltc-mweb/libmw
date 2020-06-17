#set(VCPKG_CRT_LINKAGE dynamic)
#set(VCPKG_LIBRARY_LINKAGE static)

add_definitions(-DSPDLOG_WCHAR_FILENAMES)
if (WIN32)
    #add_definitions(-DSPDLOG_WCHAR_TO_UTF8_SUPPORT)
endif()
find_package(spdlog CONFIG REQUIRED)
# target_link_libraries(main PRIVATE spdlog::spdlog spdlog::spdlog_header_only

find_package(nlohmann_json CONFIG REQUIRED)
#target_link_libraries(main PRIVATE nlohmann_json nlohmann_json::nlohmann_json)

find_package(mio CONFIG REQUIRED)
#target_link_libraries(main PRIVATE mio::mio mio::mio_base mio::mio_full_winapi)

# leveldb # TODO: Add Snappy dependency
find_package(leveldb CONFIG REQUIRED)
#target_link_libraries(main PRIVATE leveldb::leveldb)

find_package(civetweb CONFIG REQUIRED)
#target_link_libraries(main PRIVATE civetweb::civetweb civetweb::civetweb-cpp)

add_definitions(-DUNICODE)
find_package(fmt CONFIG REQUIRED)
#target_link_libraries(main PRIVATE fmt::fmt fmt::fmt-header-only)

add_definitions(-DASIO_STANDALONE)
find_package(asio CONFIG REQUIRED)
#target_link_libraries(main PRIVATE asio asio::asio)

find_package(Catch2 CONFIG REQUIRED)
#target_link_libraries(main PRIVATE Catch2::Catch2)

find_package(Boost REQUIRED)

if (MSVC)
    # VCPKG directories on Windows
    include_directories (${VCPKG_ROOT}/installed/x64-windows-static/include/)
endif (MSVC)