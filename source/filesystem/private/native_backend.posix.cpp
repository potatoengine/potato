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

#if !GM_PLATFORM_POSIX
#    error "Invalid platform"
#endif

static auto errnoToResult(int error) noexcept -> gm::fs::Result {
    switch (error) {
    case 0: return gm::fs::Result::Success;
    default: return gm::fs::Result::Unknown;
    }
}

bool gm::fs::NativeBackend::fileExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode) != 0;
}

bool gm::fs::NativeBackend::directoryExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode) != 0;
}

auto gm::fs::NativeBackend::fileStat(zstring_view path, FileStat& outInfo) const -> Result {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return errnoToResult(errno);
    }

    outInfo.size = st.st_size;
    outInfo.mtime = st.st_mtime;
    outInfo.type = S_ISREG(st.st_mode) ? FileType::Regular : S_ISDIR(st.st_mode) ? FileType::Directory : S_ISLNK(st.st_mode) ? FileType::SymbolicLink : FileType::Other;
    return Result::Success;
}

static auto enumerateWorker(gm::zstring_view path, gm::fs::EnumerateCallback cb, gm::string_writer& writer) -> gm::fs::EnumerateResult {
    gm::unique_resource<DIR*, &closedir> dir(opendir(path.c_str()));

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

        gm::fs::FileInfo info;
        info.path = writer.c_str();
        info.size = 0;
        info.type = entry->d_type == DT_REG ? gm::fs::FileType::Regular : entry->d_type == DT_DIR ? gm::fs::FileType::Directory : entry->d_type == DT_LNK ? gm::fs::FileType::SymbolicLink : gm::fs::FileType::Other;

        struct stat st;
        if (stat(writer.c_str(), &st) == 0) {
            info.size = st.st_size;
        }

        auto result = cb(info);
        if (result == gm::fs::EnumerateResult::Break) {
            return result;
        }

        if (entry->d_type == DT_DIR && result == gm::fs::EnumerateResult::Recurse) {
            auto recurse = enumerateWorker(writer.c_str(), cb, writer);
            if (recurse == gm::fs::EnumerateResult::Break) {
                return recurse;
            }
        }
    }

    return gm::fs::EnumerateResult::Continue;
}

auto gm::fs::NativeBackend::enumerate(zstring_view path, EnumerateCallback cb, EnumerateOptions opts) const -> EnumerateResult {
    string_writer writer;

    if ((opts & EnumerateOptions::FullPath) == EnumerateOptions::FullPath) {
        writer.write(path);
    }

    return enumerateWorker(path, cb, writer);
}

auto gm::fs::NativeBackend::createDirectories(zstring_view path) -> Result {
    std::string dir;

    while (!path.empty() && strcmp(path.c_str(), "/") != 0 && !directoryExists(path)) {
        if (mkdir(path.c_str(), S_IRWXU) != 0) {
            return errnoToResult(errno);
        }

        dir = gm::fs::path::parent(path);
        path = dir.c_str();
    }

    return Result::Success;
}

auto gm::fs::NativeBackend::copyFile(zstring_view from, zstring_view to) -> Result {
    gm::unique_resource<int, &close> inFile(open(from.c_str(), O_RDONLY));
    gm::unique_resource<int, &close> outFile(open(to.c_str(), O_WRONLY | O_CREAT));

    std::byte buffer[32768];

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

auto gm::fs::NativeBackend::remove(zstring_view path) -> Result {
    if (::remove(path.c_str()) != 0) {
        return errnoToResult(errno);
    }
    return Result::Success;
}

auto gm::fs::NativeBackend::removeRecursive(zstring_view path) -> Result {
    auto cb = [](char const* path, struct stat const* st, int flags, struct FTW* ftw) {
        return ::remove(path);
    };
    int rs = nftw(path.c_str(), cb, 64, FTW_DEPTH | FTW_PHYS);
    if (rs != 0) {
        return errnoToResult(errno);
    }
    return Result::Success;
}
