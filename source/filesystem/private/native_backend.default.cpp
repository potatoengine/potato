// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/native_backend.h"
#include <filesystem>

bool gm::fs::NativeBackend::fileExists(zstring_view path) const noexcept {
    return std::filesystem::is_regular_file(std::string_view(path));
}

bool gm::fs::NativeBackend::directoryExists(zstring_view path) const noexcept {
    return std::filesystem::is_directory(std::string_view(path));
}

auto gm::fs::NativeBackend::enumerate(zstring_view path, EnumerateCallback cb) const -> EnumerateResult {
    auto iter = std::filesystem::recursive_directory_iterator(path.c_str());
    auto end = std::filesystem::recursive_directory_iterator();

    while (iter != end) {
        std::string path = iter->path().generic_string().c_str();

        FileInfo info;
        info.path = path.c_str();
        info.size = iter->file_size();
        info.type = iter->is_regular_file() ? FileType::Regular : iter->is_directory() ? FileType::Directory : iter->is_symlink() ? FileType::SymbolicLink : FileType::Other;

        auto result = cb(info);
        if (result == EnumerateResult::Break) {
            return result;
        }

        if (iter->is_directory() && result == EnumerateResult::Continue) {
            iter.disable_recursion_pending();
        }

        ++iter;
    }

    return EnumerateResult::Continue;
}

bool gm::fs::NativeBackend::createDirectories(zstring_view path) {
    std::error_code ec;
    std::filesystem::create_directories(path.c_str(), ec);
    return ec == std::error_code();
}
