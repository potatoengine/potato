// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_copy.h"
#include "grimm/foundation/std_iostream.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/filesystem/filesystem.h"

gm::recon::CopyConverter::CopyConverter() = default;

gm::recon::CopyConverter::~CopyConverter() = default;

bool gm::recon::CopyConverter::convert(Context& ctx) {
    auto sourceAbsolutePath = fs::path::join({ctx.sourceFolderPath(), ctx.sourceFilePath()});
    auto destAbsolutePath = fs::path::join({ctx.destinationFolderPath(), ctx.sourceFilePath()});

    string destParentAbsolutePath(fs::path::parent(string_view(destAbsolutePath)));

    fs::FileSystem fileSys;

    if (!fileSys.directoryExists(destParentAbsolutePath.c_str())) {
        if (fileSys.createDirectories(destParentAbsolutePath.c_str()) != fs::Result::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    // output has same name as input
    ctx.addOutput(ctx.sourceFilePath());

    if (fileSys.copyFile(sourceAbsolutePath.c_str(), destAbsolutePath.c_str()) != fs::Result::Success) {
        ctx.logger().error("Failed to copy `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);
        return false;
    }

    ctx.logger().info("Copied `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);

    return true;
}
