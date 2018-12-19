#/bin/bash
set -o xtrace
export CXX=clang++-7
export CC=clang-7
export CXXFLAGS="-stdlib=libc++ -m64"
cmake /source
cmake --build . --parallel
