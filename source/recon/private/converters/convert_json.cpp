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
            std::cerr << "Failed to create `" << destParentAbsolutePath << '\n';
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    std::ifstream inFile(sourceAbsolutePath.c_str());
    if (!inFile) {
        std::cerr << "Failed to open `" << sourceAbsolutePath << "'\n";
        return false;
    }

    rapidjson::Document doc;
    rapidjson::IStreamWrapper inStream(inFile);
    doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag>(inStream);
    if (doc.HasParseError()) {
        std::cerr << "Failed to parse `" << sourceAbsolutePath << "': " << doc.GetParseError() << '\n';
        return false;
    }

    inFile.close();

    std::ofstream outFile(destAbsolutePath.c_str());
    if (!outFile) {
        std::cerr << "Failed to open `" << destAbsolutePath << "'\n";
        return false;
    }

    rapidjson::OStreamWrapper outStream(outFile);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(outStream);

    if (!doc.Accept(writer)) {
        std::cerr << "Failed to write to `" << destAbsolutePath << "'\n";
        return false;
    }

    outFile.close();

    // output has same name as input
    ctx.addOutput(ctx.sourceFilePath());

    std::cout << "Minified `" << sourceAbsolutePath << "' to `" << destAbsolutePath << "'\n";

    return true;
}
