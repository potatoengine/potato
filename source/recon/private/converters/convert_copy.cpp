// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_copy.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/filesystem/filesystem.h"
#include <iostream>

gm::recon::CopyConverter::CopyConverter() = default;

gm::recon::CopyConverter::~CopyConverter() = default;

bool gm::recon::CopyConverter::convert(Context& ctx) {
    auto sourceAbsolutePath = fs::path::join({ctx.sourceFolderPath(), ctx.sourceFilePath()});
    auto destAbsolutePath = fs::path::join({ctx.destinationFolderPath(), ctx.sourceFilePath()});

    std::string destParentAbsolutePath(fs::path::parent(string_view(destAbsolutePath)));

    fs::FileSystem fileSys;

    if (!fileSys.directoryExists(destParentAbsolutePath.c_str())) {
        if (!fileSys.createDirectories(destParentAbsolutePath.c_str())) {
            std::cerr << "Failed to create `" << destParentAbsolutePath << '\n';
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    // output has same name as input
    ctx.addOutput(ctx.sourceFilePath());

    if (fileSys.copyFile(sourceAbsolutePath.c_str(), destParentAbsolutePath.c_str())) {
        std::cerr << "Failed to coy `" << sourceAbsolutePath << "' to `" << destAbsolutePath << '\n';
        return false;
    }

    std::cout << "Copied `" << sourceAbsolutePath << "' to `" << destAbsolutePath << "'\n";

    return true;
}
