// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/native_backend.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/string_writer.h"
#include "grimm/foundation/span.h"

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

auto gm::fs::NativeBackend::enumerate(zstring_view path, EnumerateCallback cb) const -> EnumerateResult {
    gm::unique_resource<DIR*, &closedir> dir(opendir(path.c_str()));

    string_writer writer;

    for (struct dirent* entry = readdir(dir.get()); entry != nullptr; entry = readdir(dir.get())) {
        // skip . and ..
        if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        // FIXME: don't recopy bytes every iteration, and don't allocate a whole
        // path per recursive entry
        writer.clear();
        writer.write(path);
        writer.write('/');
        writer.write(entry->d_name);

        FileInfo info;
        info.path = writer.c_str();
        info.size = 0;
        info.type = entry->d_type == DT_REG ? FileType::Regular : entry->d_type == DT_DIR ? FileType::Directory : entry->d_type == DT_LNK ? FileType::SymbolicLink : FileType::Other;

        struct stat st;
        if (stat(writer.c_str(), &st) == 0) {
            info.size = st.st_size;
        }

        auto result = cb(info);
        if (result == EnumerateResult::Break) {
            return result;
        }

        if (entry->d_type == DT_DIR && result == EnumerateResult::Recurse) {
            auto recurse = enumerate(writer.c_str(), cb);
            if (recurse == EnumerateResult::Break) {
                return recurse;
            }
        }
    }

    return EnumerateResult::Continue;
}
