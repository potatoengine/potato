set -o xtrace

export CXX=clang++
export CC=clang
export CXXFLAGS="-m64 -stdlib=libc++"

cmake /source -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON

#find /source/source -name '*.cpp' | xargs -t clang-tidy -p /build -header-filter='/source/source/*'
python3 /source/extra/ci/run-clang-format.py /source/source -r --clang-format-executable=clang-format -e '*/debug.windows.h' -e '*/doctest.h'
