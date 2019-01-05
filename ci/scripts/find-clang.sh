#/bin/sh
CLANG_VERSION=7
STDLIB=libc++

set -o verbose

if [ -z "${CLANG_CC}" ] ; then
    CLANG_CC=`which clang-${CLANG_VERSION} 2>/dev/null`
    if [ -z "${CLANG_CC}" ] ; then CLANG_CC=`which clang` ; fi
fi

if [ -z "${CLANG_CXX}" ] ; then
    CLANG_CXX=`which clang++-${CLANG_VERSION} 2>/dev/null`
    if [ -z "${CLANG_CXX}" ] ; then CLANG_CXX=`which clang++` ; fi
fi

if [ -z "${CLANG_CXXFLAGS}" ] ; then CLANG_CXXFLAGS="-m64 -stdlib=${STDLIB}" ; fi

if [ -z "${CLANG_CCFLAGS}" ] ; then CLANG_CCFLAGS="-m64" ; fi
