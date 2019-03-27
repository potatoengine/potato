// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_json.h"
#include "grimm/foundation/std_iostream.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/filesystem/filesystem.h"
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>

gm::recon::JsonConverter::JsonConverter() = default;

gm::recon::JsonConverter::~JsonConverter() = default;

bool gm::recon::JsonConverter::convert(Context& ctx) {

    auto sourceAbsolutePath = fs::path::join({ctx.sourceFolderPath(), ctx.sourceFilePath()});
    auto destAbsolutePath = fs::path::join({ctx.destinationFolderPath(), ctx.sourceFilePath()});

    string destParentAbsolutePath(fs::path::parent(destAbsolutePath));

    fs::FileSystem fileSys;

    if (!fileSys.directoryExists(destParentAbsolutePath.c_str())) {
        if (fileSys.createDirectories(destParentAbsolutePath.c_str()) != fs::Result::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    std::ifstream inFile(sourceAbsolutePath.c_str());
    if (!inFile) {
        ctx.logger().error("Failed to open `{}'", sourceAbsolutePath);
        return false;
    }

    rapidjson::Document doc;
    rapidjson::IStreamWrapper inStream(inFile);
    doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag>(inStream);
    if (doc.HasParseError()) {
        ctx.logger().error("Failed to parse `{}': {}", sourceAbsolutePath, doc.GetParseError());
        return false;
    }

    inFile.close();

    std::ofstream outFile(destAbsolutePath.c_str());
    if (!outFile) {
        ctx.logger().error("Failed to open `{}'", destAbsolutePath);
        return false;
    }

    rapidjson::OStreamWrapper outStream(outFile);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(outStream);

    if (!doc.Accept(writer)) {
        ctx.logger().error("Failed to write to `{}'", destAbsolutePath);
        return false;
    }

    outFile.close();

    // output has same name as input
    ctx.addOutput(ctx.sourceFilePath());

    ctx.logger().info("Minified `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);

    return true;
}
