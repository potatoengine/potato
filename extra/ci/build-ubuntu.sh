#/bin/bash
build_type="${1:-Debug}"

set -o xtrace
export CXX=clang++-7
export CC=clang-7
export CXXFLAGS="-stdlib=libc++ -m64"
cmake /source -G Ninja -DCMAKE_BUILD_TYPE:STRING=${build_type}
cmake --build . --parallel
