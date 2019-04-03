// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/native_backend.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/string_writer.h"
#include "grimm/foundation/span.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <ftw.h>
#include <errno.h>
#include <stdio.h>

#if !UP_PLATFORM_POSIX
#    error "Invalid platform"
#endif

static auto errnoToResult(int error) noexcept -> up::fs::Result {
    switch (error) {
    case 0: return up::fs::Result::Success;
    default: return up::fs::Result::Unknown;
    }
}

bool up::fs::NativeBackend::fileExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode) != 0;
}

bool up::fs::NativeBackend::directoryExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode) != 0;
}

auto up::fs::NativeBackend::fileStat(zstring_view path, FileStat& outInfo) const -> Result {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return errnoToResult(errno);
    }

    outInfo.size = st.st_size;
    outInfo.mtime = st.st_mtime;
    outInfo.type = S_ISREG(st.st_mode) ? FileType::Regular : S_ISDIR(st.st_mode) ? FileType::Directory : S_ISLNK(st.st_mode) ? FileType::SymbolicLink : FileType::Other;
    return Result::Success;
}

static auto enumerateWorker(up::zstring_view path, up::fs::EnumerateCallback cb, up::string_writer& writer) -> up::fs::EnumerateResult {
    up::unique_resource<DIR*, &closedir> dir(opendir(path.c_str()));

    auto writerPos = writer.size();

    for (struct dirent* entry = readdir(dir.get()); entry != nullptr; entry = readdir(dir.get())) {
        // skip . and ..
        if (entry->d_type == DT_DIR && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)) {
            continue;
        }

        // FIXME: don't recopy bytes every iteration, and don't allocate a whole
        // path per recursive entry
        writer.resize(writerPos);
        if (!writer.empty()) {
            writer.write('/');
        }
        writer.write(entry->d_name);

        up::fs::FileInfo info;
        info.path = writer.c_str();
        info.size = 0;
        info.type = entry->d_type == DT_REG ? up::fs::FileType::Regular : entry->d_type == DT_DIR ? up::fs::FileType::Directory : entry->d_type == DT_LNK ? up::fs::FileType::SymbolicLink : up::fs::FileType::Other;

        struct stat st;
        if (stat(writer.c_str(), &st) == 0) {
            info.size = st.st_size;
        }

        auto result = cb(info);
        if (result == up::fs::EnumerateResult::Break) {
            return result;
        }

        if (entry->d_type == DT_DIR && result == up::fs::EnumerateResult::Recurse) {
            auto recurse = enumerateWorker(writer.c_str(), cb, writer);
            if (recurse == up::fs::EnumerateResult::Break) {
                return recurse;
            }
        }
    }

    return up::fs::EnumerateResult::Continue;
}

auto up::fs::NativeBackend::enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts) const -> EnumerateResult {
    string_writer writer;

    if ((opts & EnumerateOptions::FullPath) == EnumerateOptions::FullPath) {
        writer.write(path);
    }

    return enumerateWorker(path, cb, writer);
}

auto up::fs::NativeBackend::createDirectories(zstring_view path) -> Result {
    string dir;

    while (!path.empty() && strcmp(path.c_str(), "/") != 0 && !directoryExists(path)) {
        if (mkdir(path.c_str(), S_IRWXU) != 0) {
            return errnoToResult(errno);
        }

        dir = up::fs::path::parent(path);
        path = dir.c_str();
    }

    return Result::Success;
}

auto up::fs::NativeBackend::copyFile(zstring_view from, zstring_view to) -> Result {
    up::unique_resource<int, &close> inFile(open(from.c_str(), O_RDONLY));
    up::unique_resource<int, &close> outFile(open(to.c_str(), O_WRONLY | O_CREAT, S_IRWXU));

    up::byte buffer[32768];

    for (;;) {
        ssize_t rs = read(inFile.get(), buffer, sizeof(buffer));
        if (rs < 0) {
            return errnoToResult(errno);
        }

        if (rs == 0) {
            return Result::Success;
        }

        ssize_t rs2 = write(outFile.get(), buffer, rs);
        if (rs2 != rs) {
            return errnoToResult(errno);
        }
    }
}

auto up::fs::NativeBackend::remove(zstring_view path) -> Result {
    if (::remove(path.c_str()) != 0) {
        return errnoToResult(errno);
    }
    return Result::Success;
}

auto up::fs::NativeBackend::removeRecursive(zstring_view path) -> Result {
    auto cb = [](char const* path, struct stat const* st, int flags, struct FTW* ftw) -> int {
        return ::remove(path);
    };
    int rs = nftw(path.c_str(), +cb, 64, FTW_DEPTH | FTW_PHYS);
    if (rs != 0) {
        return errnoToResult(errno);
    }
    return Result::Success;
}
