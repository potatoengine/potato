FROM ubuntu:16.04

# Minimum requirements just to get rolling
RUN apt-get update
RUN apt-get install -y --no-install-recommends apt-transport-https gnupg2 ca-certificates

# Extra repositories and their keys
COPY apt-public-keys.gpg /root
RUN apt-key add /root/apt-public-keys.gpg && rm /root/apt-public-keys.gpg

# Modernized build environment
COPY apt-sources-xenial.list /etc/apt/sources.list.d/
RUN apt-get -y update
RUN apt-get -y --no-install-recommends install cmake ninja-build
RUN apt-get -y --no-install-recommends install clang-7 libc++-dev libc++abi-dev
RUN apt-get -y --no-install-recommends install clang-tidy-7 clang-format-7

# Build dependencies for graphics/games - sadly SDL2 heavily bloats the image because it pulls in way too much crap
RUN apt-get -y --no-install-recommends install vulkan-headers
RUN apt-get -y --no-install-recommends install libsdl2-dev

# Setup to run
RUN mkdir /build
ENV HOME /build
WORKDIR /build
CMD ["bash"]
