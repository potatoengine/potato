// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_json.h"
#include "potato/spud/std_iostream.h"
#include "potato/runtime/path.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/logger.h"
#include <fstream>
#include <nlohmann/json.hpp>

up::recon::JsonConverter::JsonConverter() = default;

up::recon::JsonConverter::~JsonConverter() = default;

bool up::recon::JsonConverter::convert(Context& ctx) {

    auto sourceAbsolutePath = path::join({ctx.sourceFolderPath(), ctx.sourceFilePath()});
    auto destAbsolutePath = path::join({ctx.destinationFolderPath(), ctx.sourceFilePath()});

    string destParentAbsolutePath(path::parent(destAbsolutePath));

    if (!ctx.fileSystem().directoryExists(destParentAbsolutePath.c_str())) {
        if (ctx.fileSystem().createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    std::ifstream inFile(sourceAbsolutePath.c_str());
    if (!inFile) {
        ctx.logger().error("Failed to open `{}'", sourceAbsolutePath);
        return false;
    }

    nlohmann::json doc = nlohmann::json::parse(inFile, nullptr, false);
    if (doc.is_discarded()) {
        ctx.logger().error("Failed to parse `{}': {}", sourceAbsolutePath, "unknown parse error");
        return false;
    }

    inFile.close();

    std::ofstream outFile(destAbsolutePath.c_str());
    if (!outFile) {
        ctx.logger().error("Failed to open `{}'", destAbsolutePath);
        return false;
    }

    outFile << doc;

    if (!outFile) {
        ctx.logger().error("Failed to write to `{}'", destAbsolutePath);
        return false;
    }

    outFile.close();

    // output has same name as input
    ctx.addOutput(ctx.sourceFilePath());

    ctx.logger().info("Minified `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);

    return true;
}
