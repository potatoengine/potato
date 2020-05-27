// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "convert_copy.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/path.h"
#include "potato/spud/std_iostream.h"

up::recon::CopyConverter::CopyConverter() = default;

up::recon::CopyConverter::~CopyConverter() = default;

bool up::recon::CopyConverter::convert(Context& ctx) {
    auto sourceAbsolutePath = path::join({ctx.sourceFolderPath(), ctx.sourceFilePath()});
    auto destAbsolutePath = path::join({ctx.destinationFolderPath(), ctx.sourceFilePath()});

    string destParentAbsolutePath(path::parent(string_view(destAbsolutePath)));

    if (!ctx.fileSystem().directoryExists(destParentAbsolutePath.c_str())) {
        if (ctx.fileSystem().createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    // output has same name as input
    ctx.addOutput(ctx.sourceFilePath());

    auto rs = ctx.fileSystem().copyFile(sourceAbsolutePath.c_str(), destAbsolutePath.c_str());
    if (rs != IOResult::Success) {
        ctx.logger().error("Failed to copy `{}' to `{}': {}", sourceAbsolutePath, destAbsolutePath, static_cast<uint64>(rs));
        return false;
    }

    ctx.logger().info("Copied `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);

    return true;
}
