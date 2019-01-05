#/bin/sh
BUILD_TYPE="${1:-Debug}"
DIR=`dirname "$0"`

. "${DIR}/find-clang.sh"

set -o xtrace

export CXX="${CLANG_CXX}"
export CC="${CLANG_CC}"
export CXXFLAGS="${CLANG_CXXFLAGS}"
export CCFLAGS="${CLANG_CCFLAGS}"

cmake /source -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -DCMAKE_BUILD_TYPE:STRING="${BUILD_TYPE}" -GNinja
