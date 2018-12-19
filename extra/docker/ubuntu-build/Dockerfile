FROM ubuntu:16.04

# Minimum requirements just to get rolling
RUN apt-get update
RUN apt-get install -y wget software-properties-common apt-transport-https

# Extra repositories and their keys
RUN wget -O - "https://apt.llvm.org/llvm-snapshot.gpg.key" | apt-key add -
RUN wget -qO - "http://packages.lunarg.com/lunarg-signing-key-pub.asc" | apt-key add -
RUN . /etc/lsb-release && wget -qO "/etc/apt/sources.list.d/lunarg-vulkan-1.1.92-${DISTRIB_CODENAME}.list" "http://packages.lunarg.com/vulkan/1.1.92/lunarg-vulkan-1.1.92-${DISTRIB_CODENAME}.list"
RUN . /etc/lsb-release && add-apt-repository -y "deb http://apt.llvm.org/${DISTRIB_CODENAME}/ llvm-toolchain-${DISTRIB_CODENAME}-7 main"

# Modernized build environment
RUN add-apt-repository -y ppa:janisozaur/cmake-update
RUN apt-get -y update
RUN apt-get -y install cmake clang-7 libc++-7-dev libc++abi-7-dev ninja-build
RUN apt-get -y install clang-tidy-7 clang-format-7

# Build dependencies
RUN apt-get -y install lunarg-vulkan-sdk libsdl2-dev

# Setup to run
RUN mkdir /build
ENV HOME /build
WORKDIR /build
CMD ["bash"]
