// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/filesystem.h"
#include "grimm/foundation/platform.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if !GM_PLATFORM_UNIX
#    error "Invalid platform"
#endif

bool gm::fs::FileSystem::fileExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFREG) != 0;
}

bool gm::fs::FileSystem::directoryExists(zstring_view path) const noexcept {
    struct stat st;
    if (::stat(path.c_str(), &st) != 0) {
        return false;
    }
    return (st.st_mode & S_IFDIR) != 0;
}
