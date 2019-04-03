// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/null_backend.h"
#include "grimm/filesystem/stream.h"

bool gm::fs::NullBackend::fileExists(zstring_view path) const noexcept { return false; }

bool gm::fs::NullBackend::directoryExists(zstring_view path) const noexcept { return false; }

auto gm::fs::NullBackend::fileStat(zstring_view path, FileStat& outInfo) const -> Result { return Result::UnsupportedOperation; }

auto gm::fs::NullBackend::openRead(zstring_view, FileOpenMode) const -> Stream { return {}; }

auto gm::fs::NullBackend::openWrite(zstring_view, FileOpenMode) -> Stream { return {}; }

auto gm::fs::NullBackend::enumerate(zstring_view, EnumerateCallback, EnumerateOptions) const -> EnumerateResult { return EnumerateResult::Continue; }

auto gm::fs::NullBackend::createDirectories(zstring_view path) -> Result { return Result::UnsupportedOperation; }

auto gm::fs::NullBackend::copyFile(zstring_view from, zstring_view to) -> Result { return Result::UnsupportedOperation; }

auto gm::fs::NullBackend::remove(zstring_view path) -> Result { return Result::UnsupportedOperation; }

auto gm::fs::NullBackend::removeRecursive(zstring_view path) -> Result { return Result::UnsupportedOperation; }
