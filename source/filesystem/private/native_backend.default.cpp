// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/native_backend.h"
#include <filesystem>

bool gm::fs::NativeBackend::fileExists(zstring_view path) const noexcept {
    return std::filesystem::is_regular_file(std::string_view(path));
}

bool gm::fs::NativeBackend::directoryExists(zstring_view path) const noexcept {
    return std::filesystem::is_directory(std::string_view(path));
}

namespace {
    struct DefaultDirectoryIterator : gm::fs::DirectoryIteratorBackend {
        std::filesystem::recursive_directory_iterator iter;
        std::string path;

        DefaultDirectoryIterator(gm::zstring_view path) : iter(std::filesystem::recursive_directory_iterator(path.c_str())) {
            next();
        }

        bool next() {
            if (++iter == std::filesystem::recursive_directory_iterator()) {
                return false;
            }

            path = std::move(iter->path().generic_string());
            return true;
        }

        bool done() const noexcept {
            return iter == std::filesystem::recursive_directory_iterator();
        }

        gm::zstring_view current() const noexcept {
            return path.c_str();
        }
    };
} // namespace

auto gm::fs::NativeBackend::recursiveEnumerate(zstring_view path) const -> DirectoryIterator {
    return DirectoryIterator(make_box<DefaultDirectoryIterator>(path));
}
