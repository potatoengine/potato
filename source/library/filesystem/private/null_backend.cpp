// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/null_backend.h"
#include "potato/filesystem/stream.h"

bool up::NullBackend::fileExists(zstring_view path) const noexcept { return false; }

bool up::NullBackend::directoryExists(zstring_view path) const noexcept { return false; }

auto up::NullBackend::fileStat(zstring_view path, FileStat& outInfo) const -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullBackend::openRead(zstring_view, FileOpenMode) const -> Stream { return {}; }

auto up::NullBackend::openWrite(zstring_view, FileOpenMode) -> Stream { return {}; }

auto up::NullBackend::enumerate(zstring_view, EnumerateCallback, EnumerateOptions) const -> EnumerateResult { return EnumerateResult::Continue; }

auto up::NullBackend::createDirectories(zstring_view path) -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullBackend::copyFile(zstring_view from, zstring_view to) -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullBackend::remove(zstring_view path) -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullBackend::removeRecursive(zstring_view path) -> IOResult { return IOResult::UnsupportedOperation; }
