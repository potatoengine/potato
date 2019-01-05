#/bin/bash
build_type="${1:-Debug}"

set -o xtrace

. "$(dirname "$0")/find-clang.sh"
export CXX="${CLANG_CXX}"
export CC="${CLANG_CC}"
export CXXFLAGS="${CLANG_CXXFLAGS}"
export CCFLAGS="${CLANG_CCFLAGS}"

cmake /source -G Ninja -DCMAKE_BUILD_TYPE:STRING="${build_type}"
cmake --build . --parallel
