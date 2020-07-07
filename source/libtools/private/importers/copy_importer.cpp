// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "copy_importer.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/path.h"
#include "potato/spud/std_iostream.h"

up::CopyImporter::CopyImporter() = default;

up::CopyImporter::~CopyImporter() = default;

bool up::CopyImporter::import(ImporterContext& ctx) {
    auto sourceAbsolutePath = path::join(ctx.sourceFolderPath(), ctx.sourceFilePath());
    auto destAbsolutePath = path::join(ctx.destinationFolderPath(), ctx.sourceFilePath());

    string destParentAbsolutePath(path::parent(string_view(destAbsolutePath)));

    if (!fs::directoryExists(destParentAbsolutePath.c_str())) {
        if (fs::createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    // output has same name as input
    ctx.addMainOutput(ctx.sourceFilePath());

    auto rs = fs::copyFileTo(sourceAbsolutePath.c_str(), destAbsolutePath.c_str());
    if (rs != IOResult::Success) {
        ctx.logger().error("Failed to copy `{}' to `{}': {}", sourceAbsolutePath, destAbsolutePath, static_cast<uint64>(rs));
        return false;
    }

    ctx.logger().info("Copied `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);

    return true;
}
