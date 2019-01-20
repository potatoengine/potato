// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/native_backend.h"
#include <filesystem>

static auto errorCodeToResult(std::error_code ec) noexcept -> gm::fs::Result {
    if (!ec) {
        return gm::fs::Result::Success;
    }

    if (ec.category() == std::system_category()) {
        // FIXME: translate error codes
        return gm::fs::Result::System;
    }
    return gm::fs::Result::Unknown;
}

bool gm::fs::NativeBackend::fileExists(zstring_view path) const noexcept {
    return std::filesystem::is_regular_file(std::string_view(path));
}

bool gm::fs::NativeBackend::directoryExists(zstring_view path) const noexcept {
    return std::filesystem::is_directory(std::string_view(path));
}

auto gm::fs::NativeBackend::fileStat(zstring_view path, FileStat& outInfo) const -> Result {
    std::error_code ec;
    outInfo.size = std::filesystem::file_size(std::string_view(path), ec);
    outInfo.mtime = std::chrono::duration_cast<std::chrono::microseconds>(std::filesystem::last_write_time(std::string_view(path), ec).time_since_epoch()).count();
    auto status = std::filesystem::status(std::string_view(path), ec);
    outInfo.type =
        status.type() == std::filesystem::file_type::regular ? FileType::Regular : status.type() == std::filesystem::file_type::directory ? FileType::Directory : status.type() == std::filesystem::file_type::symlink ? FileType::SymbolicLink : FileType::Other;
    return errorCodeToResult(ec);
}

auto gm::fs::NativeBackend::enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts) const -> EnumerateResult {
    auto iter = std::filesystem::recursive_directory_iterator(path.c_str());
    auto end = std::filesystem::recursive_directory_iterator();

    while (iter != end) {
        std::string genPath =
            (((opts & EnumerateOptions::FullPath) == EnumerateOptions::FullPath) ? iter->path() : std::filesystem::relative(iter->path(), path.c_str())).generic_string().c_str();

        FileInfo info;
        info.path = genPath.c_str();
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

auto gm::fs::NativeBackend::createDirectories(zstring_view path) -> Result {
    std::error_code ec;
    std::filesystem::create_directories(path.c_str(), ec);
    return errorCodeToResult(ec);
}

auto gm::fs::NativeBackend::copyFile(zstring_view from, zstring_view to) -> Result {
    std::error_code ec;
    std::filesystem::copy_file(from.c_str(), to.c_str(), ec);
    return errorCodeToResult(ec);
}

auto gm::fs::NativeBackend::remove(zstring_view path) -> Result {
    std::error_code ec;
    std::filesystem::remove(path.c_str(), ec);
    return errorCodeToResult(ec);
}

auto gm::fs::NativeBackend::removeRecursive(zstring_view path) -> Result {
    std::error_code ec;
    std::filesystem::remove_all(path.c_str(), ec);
    return errorCodeToResult(ec);
}
