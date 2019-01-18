// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/native_backend.h"

auto gm::fs::NativeBackend::create() -> FileSystem {
    return FileSystem(rc<NativeBackend>(new NativeBackend));
}

std::ifstream gm::fs::NativeBackend::openRead(zstring_view path, FileOpenMode mode) const {
    return std::ifstream(path.c_str(), mode == FileOpenMode::Binary ? std::ios_base::binary : 0);
}

std::ofstream gm::fs::NativeBackend::openWrite(zstring_view path, FileOpenMode mode) {
    return std::ofstream(path.c_str(), mode == FileOpenMode::Binary ? std::ios_base::binary : 0);
}
