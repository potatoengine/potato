#/bin/bash
build_type="${1:-Debug}"

set -o xtrace

export CXX=clang++
export CC=clang
export CXXFLAGS="-m64 -stdlib=libc++"

cmake /source -G Ninja -DCMAKE_BUILD_TYPE:STRING=${build_type}
cmake --build . --parallel
