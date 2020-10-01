## Build Instructions

### Pre-requisites
1. CMake 3.8+
2. Compiler with C++ 17 support (we recommend g++ 9 or later)

### Building (Linux)

1. `git clone https://github.com/GrinPlusPlus/libmw-ltc.git --recursive`
2. `cd libmw-ltc/vcpkg/vcpkg && ./bootstrap-vcpkg.sh`
3. `./vcpkg install --triplet x64-linux @../packages.txt`
4. `cd ../.. && mkdir -p build && cd build`
5. `cmake -DCMAKE_BUILD_TYPE=Release .. && sudo cmake --build . --target install`

This should install the libmw headers in `/usr/local/include/libmw`, and the dylib in `/usr/local/lib`, allowing them to be found during when building the litecoin project.