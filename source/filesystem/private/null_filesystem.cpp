// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/filesystem/null_filesystem.h"

gm::fs::NullFileSystemBackend::NullFileSystemBackend() noexcept {}

bool gm::fs::NullFileSystemBackend::fileExists(string_view path) const noexcept { return false; }

bool gm::fs::NullFileSystemBackend::directoryExists(string_view path) const noexcept { return false; }
