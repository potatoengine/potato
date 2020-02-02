// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/runtime/null.h"
#include "potato/runtime/stream.h"

bool up::NullFileSystem::fileExists(zstring_view path) const noexcept { return false; }

bool up::NullFileSystem::directoryExists(zstring_view path) const noexcept { return false; }

auto up::NullFileSystem::fileStat(zstring_view path, FileStat& outInfo) const noexcept -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::openRead(zstring_view, FileOpenMode) const noexcept -> Stream { return {}; }

auto up::NullFileSystem::openWrite(zstring_view, FileOpenMode) noexcept -> Stream { return {}; }

auto up::NullFileSystem::enumerate(zstring_view, EnumerateCallback, EnumerateOptions) const noexcept -> EnumerateResult { return EnumerateResult::Continue; }

auto up::NullFileSystem::createDirectories(zstring_view path) noexcept -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::copyFile(zstring_view from, zstring_view to) noexcept -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::remove(zstring_view path) noexcept -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::removeRecursive(zstring_view path) noexcept -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::currentWorkingDirectory() const noexcept -> string { return {}; }

auto up::NullFileSystem::currentWorkingDirectory(zstring_view) -> bool { return false; }
