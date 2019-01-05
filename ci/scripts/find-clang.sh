CLANG_VERSION=7
STDLIB=libc++

CLANG_CC=`which clang-${CLANG_VERSION}`
if [ -n "${CLANG_CC}"] ; then CLANG_CC=`which clang` ; fi

CLANG_CXX=`which clang++-${CLANG_VERSION}`
if [ -n "${CLANG_CXX}"] ; then CLANG_CXX=`which clang++` ; fi

CLANG_FORMAT=`which clang-format-${CLANG_VERSION}`
if [ -n "${CLANG_FORMAT}"] ; then CLANG_FORMAT=`which clang-format` ; fi

CLANG_CXXFLAGS="-m64 -stdlib=${STDLIB}"
CLANG_CCFLAGS="-m64"
