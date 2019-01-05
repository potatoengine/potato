#/bin/bash
CLANG_VERSION=7
STDLIB=libc++

CLANG_PATH=`which clang-${CLANG_VERSION}`
if [ -n "$CLANG_PATH"] ; then CLANG_PATH=`which clang` ; fi

CLANGXX_PATH=`which clang++-${CLANG_VERSION}`
if [ -n "$CLANGXX_PATH"] ; then CLANGXX_PATH=`which clang++` ; fi

CLANG_FORMAT_PATH=`which clang-format-${CLANG_VERSION}`
if [ -n "CLANG_FORMAT_PATH"] ; then CLANG_FORMAT_PATH=`which clang-format` ; fi

set -o xtrace

export CXX="${CLANGXX_PATH}"
export CC="${CLANG_PATH}"
export CXXFLAGS="-m64 -stdlib=${STDLIB}"

cmake /source -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON

#find /source/source -name '*.cpp' | xargs -t clang-tidy-${CLANG_VERSION} -p /build -header-filter='/source/source/*'

python /source/extra/ci/run-clang-format.py /source/source -r --clang-format-executable="${CLANG_FORMAT_PATH}" -e '*/debug.windows.h' -e 'external/'
