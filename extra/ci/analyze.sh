CLANG_VERSION=7

set -o xtrace

export CXX=clang++-${CLANG_VERSION}
export CC=clang-${CLANG_VERSION}
export CXXFLAGS="-m64 -stdlib=libc++"

cmake /source -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON

#find /source/source -name '*.cpp' | xargs -t clang-tidy-${CLANG_VERSION} -p /build -header-filter='/source/source/*'
python /source/extra/ci/run-clang-format.py /source/source -r --clang-format-executable=clang-format-${CLANG_VERSION} -e '*/debug.windows.h' -e '*/doctest.h'
