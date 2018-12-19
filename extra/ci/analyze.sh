set -o xtrace

export CXX=clang++-7
export CC=clang-7
export CXXFLAGS="-stdlib=libc++ -m64"
cmake /source -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON

#find /source/source -name '*.cpp' | xargs -t clang-tidy-7 -p /build -header-filter='/source/source/*'
python /source/extra/ci/run-clang-format.py /source/source -r --clang-format-executable=clang-format-7 -e '*/debug.windows.h' -e '*/doctest.h'
