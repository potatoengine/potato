// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/null.h"
#include "potato/filesystem/stream.h"

bool up::NullFileSystem::fileExists(zstring_view path) const noexcept { return false; }

bool up::NullFileSystem::directoryExists(zstring_view path) const noexcept { return false; }

auto up::NullFileSystem::fileStat(zstring_view path, FileStat& outInfo) const -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::openRead(zstring_view, FileOpenMode) const -> Stream { return {}; }

auto up::NullFileSystem::openWrite(zstring_view, FileOpenMode) -> Stream { return {}; }

auto up::NullFileSystem::enumerate(zstring_view, EnumerateCallback, EnumerateOptions) const -> EnumerateResult { return EnumerateResult::Continue; }

auto up::NullFileSystem::createDirectories(zstring_view path) -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::copyFile(zstring_view from, zstring_view to) -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::remove(zstring_view path) -> IOResult { return IOResult::UnsupportedOperation; }

auto up::NullFileSystem::removeRecursive(zstring_view path) -> IOResult { return IOResult::UnsupportedOperation; }
