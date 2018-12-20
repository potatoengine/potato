#/bin/bash
build_type="${1:-Debug}"
CLANG_VERSION=7

set -o xtrace

export CXX=clang++-${CLANG_VERSION}
export CC=clang-${CLANG_VERSION}
export CXXFLAGS="-m64 -stdlib=libc++"

cmake /source -G Ninja -DCMAKE_BUILD_TYPE:STRING=${build_type}
cmake --build . --parallel
