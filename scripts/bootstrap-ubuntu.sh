SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
ROOT="$( dirname "$SCRIPT_DIR" )"
BUILD_DIR="${ROOT}/build/ubuntu"

. /etc/lsb-release

wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -

sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
sudo add-apt-repository -y ppa:janisozaur/cmake-update
sudo add-apt-repository -y "deb http://apt.llvm.org/${DISTRIB_CODENAME}/ llvm-toolchain-${DISTRIB_CODENAME}-7 main"

sudo apt-get -y update
sudo apt-get -y install cmake clang-7 libc++-7-dev libc++abi-7-dev g++-7 ninja-build unzip

export CXX=g++-7

# vcpkg only compiles with g++, and SDL2 is a C library so any compiler works
bash "$ROOT/vcpkg/bootstrap-vcpkg.sh"
"$ROOT/vcpkg/vcpkg" install sdl2:x64-linux 

export CXX=clang++-7
export CXXFLAGS="-m64 -stdlib=libc++"

# fmt is mostly header-only so it needs to use the same C++ library
"$ROOT/vcpkg/vcpkg" install fmt:x64-linux

mkdir -p "$BUILD_DIR/RelWithDebInfo" && cd "$BUILD_DIR/RelWithDebInfo"
/usr/bin/cmake -G Ninja -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo -DCMAKE_TOOLCHAIN_FILE="$ROOT/vcpkg/scripts/buildsystems/vcpkg.cmake" "$ROOT"

mkdir -p "$BUILD_DIR/Debug" && cd "$BUILD_DIR/Debug"
/usr/bin/cmake -G Ninja -DCMAKE_BUILD_TYPE:STRING=Debug -DCMAKE_TOOLCHAIN_FILE="$ROOT/vcpkg/scripts/buildsystems/vcpkg.cmake" "$ROOT"