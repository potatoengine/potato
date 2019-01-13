// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/filesystem.h"
#include <filesystem>

bool gm::fs::FileSystem::fileExists(zstring_view path) const noexcept {
    return std::filesystem::is_regular_file(std::string_view(path));
}

bool gm::fs::FileSystem::directoryExists(zstring_view path) const noexcept {
    return std::filesystem::is_directory(std::string_view(path));
}
