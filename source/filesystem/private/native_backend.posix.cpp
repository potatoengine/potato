// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/native_backend.h"
#include "grimm/filesystem/directory_iterator.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/string_writer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#if !GM_PLATFORM_POSIX
#    error "Invalid platform"
#endif

bool gm::fs::NativeBackend::fileExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFREG) != 0;
}

bool gm::fs::NativeBackend::directoryExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFDIR) != 0;
}

namespace {
    struct PosixDirectoryIterator : public gm::fs::DirectoryIteratorBackend {
        struct State {
            gm::unique_resource<DIR*, &closedir> dir;
            struct dirent* entry = nullptr;
        };

        PosixDirectoryIterator(zstring_view path) {
            dir.reset(opendir(name.c_str());
            auto entry = readdir(dir.get());
            stack.emplace_back(std::move(dir), entry);

            // FIXME: skip . and ..
        }

        bool next() override {
            State& curr = stack.back();
            if (curr.entr->d_type == DT_DIR) {
                            dir.reset(opendir(name.c_str());
            auto entry = readdir(dir.get());
            stack.emplace_back(std::move(dir), entry);
            return true;
            }

            curr.entry = readdir(curr.dir.get());
            while (curr.entry == nullptr) {
                stack.pop_back();
                if (stack.empty()) {
                    return false;
                }
                curr.entry = readdir(curr.dir.get());
            }

            return true;
            
            //if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
        }

        bool done() const noexcept override {
            return stack.empty();
        }

        gm::vector<State> stack;
    };
} // namespace

auto gm::fs::NativeBackend::recursiveEnumerate(zstring_view path) const -> DirectoryIterator {
    return DirectoryIterator(make_box<PosixDirectoryIterator>(path));
}
