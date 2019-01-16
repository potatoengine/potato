// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/null_backend.h"

bool gm::fs::NullBackend::fileExists(zstring_view path) const noexcept { return false; }

bool gm::fs::NullBackend::directoryExists(zstring_view path) const noexcept { return false; }

std::ifstream gm::fs::NullBackend::openRead(zstring_view path) const { return {}; }

std::ofstream gm::fs::NullBackend::openWrite(zstring_view path) { return {}; }

auto gm::fs::NullBackend::enumerate(zstring_view path, EnumerateCallback cb) const -> EnumerateResult { return EnumerateResult::Continue; }

bool gm::fs::NullBackend::createDirectories(zstring_view path) { return false; }

bool gm::fs::NullBackend::copyFile(zstring_view from, zstring_view to) { return false; }
