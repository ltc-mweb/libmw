#set(VCPKG_CRT_LINKAGE dynamic)
#set(VCPKG_LIBRARY_LINKAGE static)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(mio CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem thread)