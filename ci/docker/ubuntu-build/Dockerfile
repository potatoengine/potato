FROM ubuntu:18.10

# Files needed for configuration
COPY libgl1-mesa-dri.control /root

# Install dependencies
RUN apt-get update -y && \
    apt-get -y --no-install-recommends install cmake && \
    apt-get -y --no-install-recommends install ninja-build && \
    apt-get -y --no-install-recommends install clang-7 && \
    apt-get -y --no-install-recommends install libc++-7-dev libc++abi-7-dev && \
    apt-get -y --no-install-recommends install clang-tidy-7 clang-format-7 && \
    apt-get -y --no-install-recommends install clang-format-7 && \
    apt-get -y --no-install-recommends install equivs && \
    cd /root && equivs-build /root/libgl1-mesa-dri.control && \
    dpkg -i /root/libgl1-mesa-dri_1.0_all.deb && \
    apt-get -y --no-install-recommends install libvulkan-dev && \
    apt-get -y --no-install-recommends install libsdl2-dev && \
    apt-get clean && \
    mkdir /build

# Setup to run
ENV HOME /build
WORKDIR /build
CMD ["bash"]
