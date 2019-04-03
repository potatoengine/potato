// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/null_backend.h"
#include "grimm/filesystem/stream.h"

bool up::fs::NullBackend::fileExists(zstring_view path) const noexcept { return false; }

bool up::fs::NullBackend::directoryExists(zstring_view path) const noexcept { return false; }

auto up::fs::NullBackend::fileStat(zstring_view path, FileStat& outInfo) const -> Result { return Result::UnsupportedOperation; }

auto up::fs::NullBackend::openRead(zstring_view, FileOpenMode) const -> Stream { return {}; }

auto up::fs::NullBackend::openWrite(zstring_view, FileOpenMode) -> Stream { return {}; }

auto up::fs::NullBackend::enumerate(zstring_view, EnumerateCallback, EnumerateOptions) const -> EnumerateResult { return EnumerateResult::Continue; }

auto up::fs::NullBackend::createDirectories(zstring_view path) -> Result { return Result::UnsupportedOperation; }

auto up::fs::NullBackend::copyFile(zstring_view from, zstring_view to) -> Result { return Result::UnsupportedOperation; }

auto up::fs::NullBackend::remove(zstring_view path) -> Result { return Result::UnsupportedOperation; }

auto up::fs::NullBackend::removeRecursive(zstring_view path) -> Result { return Result::UnsupportedOperation; }
