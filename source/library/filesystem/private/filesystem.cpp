// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/backend.h"
#include "grimm/filesystem/native_backend.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/stream.h"

gm::fs::FileSystem::FileSystem() : _impl(activeDefaultBackend()) {}
gm::fs::FileSystem::FileSystem(rc<Backend> backend) : _impl(std::move(backend)) {}

gm::fs::FileSystem::~FileSystem() = default;

auto gm::fs::FileSystem::swapDefaultBackend(rc<Backend> backend) -> rc<Backend> {
    auto& active = activeDefaultBackend();
    std::swap(active, backend);
    return backend;
}

auto gm::fs::FileSystem::activeDefaultBackend() -> rc<Backend>& {
    static rc<Backend> active(rc<NativeBackend>(new NativeBackend));
    return active;
}

auto gm::fs::FileSystem::openRead(zstring_view path, FileOpenMode mode) const noexcept -> Stream {
    return _impl->openRead(path, mode);
}

auto gm::fs::FileSystem::openWrite(zstring_view path, FileOpenMode mode) const noexcept -> Stream {
    return _impl->openWrite(path, mode);
}