FROM ubuntu:16.04

# Minimum requirements just to get rolling
RUN apt-get update -y
RUN apt-get install -y --no-install-recommends apt-transport-https gnupg2 ca-certificates

# Extra repositories and their keys
COPY apt-public-keys.gpg /root
COPY apt-sources-xenial.list /etc/apt/sources.list.d/
RUN apt-key add /root/apt-public-keys.gpg && rm /root/apt-public-keys.gpg
RUN apt-get update -y

# Modernized build environment
RUN apt-get -y --no-install-recommends install cmake ninja-build
RUN apt-get -y --no-install-recommends install clang-7 libc++-7-dev libc++abi-7-dev
RUN apt-get -y --no-install-recommends install clang-tidy-7 clang-format-7

# Fake GL drivers because otherwise libsdl2 pulls in hundreds of MB
COPY libgl1-mesa-dri.control /root
RUN apt-get -y --no-install-recommends install equivs
RUN equivs-build /root/libgl1-mesa-dri.control
RUN dpkg -i /root/libgl1-mesa-dri_1.0_all.dpkg

# Build dependencies for graphics/games
RUN apt-get -y --no-install-recommends install libvulkan-dev
RUN apt-get -y --no-install-recommends install libsdl2-dev

# Setup to run
RUN mkdir /build
ENV HOME /build
WORKDIR /build
CMD ["bash"]
