FROM fedora:29

# Modernized build environment
RUN dnf -y install which --setopt=install_weak_deps=False && \
    dnf -y install cmake --setopt=install_weak_deps=False && \
    dnf -y install ninja-build --setopt=install_weak_deps=False && \
    dnf -y install libcxx-devel libcxx-static libcxxabi-devel --setopt=install_weak_deps=False && \
    dnf -y install clang --setopt=install_weak_deps=False && \
    dnf -y install vulkan-headers vulkan-loader --setopt=install_weak_deps=False && \
    dnf -y install SDL2-devel SDL2-static --setopt=install_weak_deps=False && \
    dnf -y install assimp assimp-devel --setopt=install_weak_deps=False && \
    dnf clean all

# Setup to run
RUN mkdir /build
ENV HOME /build
WORKDIR /build
CMD ["bash"]
