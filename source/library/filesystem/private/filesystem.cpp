// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/backend.h"
#include "potato/filesystem/native_backend.h"
#include "potato/filesystem/filesystem.h"
#include "potato/filesystem/stream.h"

up::FileSystem::FileSystem() : _impl(activeDefaultBackend()) {}
up::FileSystem::FileSystem(rc<Backend> backend) : _impl(std::move(backend)) {}

up::FileSystem::~FileSystem() = default;

auto up::FileSystem::swapDefaultBackend(rc<Backend> backend) -> rc<Backend> {
    auto& active = activeDefaultBackend();
    std::swap(active, backend);
    return backend;
}

auto up::FileSystem::activeDefaultBackend() -> rc<Backend>& {
    static rc<Backend> active(rc<NativeBackend>(new NativeBackend));
    return active;
}

auto up::FileSystem::openRead(zstring_view path, FileOpenMode mode) const noexcept -> Stream {
    return _impl->openRead(path, mode);
}

auto up::FileSystem::openWrite(zstring_view path, FileOpenMode mode) const noexcept -> Stream {
    return _impl->openWrite(path, mode);
}
