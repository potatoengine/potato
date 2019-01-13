// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_json.h"
#include <iostream>
#include <fstream>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>

gm::recon::JsonConverter::JsonConverter() = default;

gm::recon::JsonConverter::~JsonConverter() = default;

bool gm::recon::JsonConverter::convert(Context& ctx) {

    std::filesystem::path sourceAbsolutePath = ctx.sourceFolderPath() / ctx.sourceFilePath();
    std::filesystem::path destAbsolutePath = ctx.destinationFolderPath() / ctx.sourceFilePath();

    std::filesystem::path destParentAbsolutePath = destAbsolutePath.parent_path();

    std::error_code rs;

    if (!std::filesystem::is_directory(destParentAbsolutePath)) {
        if (!std::filesystem::create_directories(destParentAbsolutePath, rs)) {
            std::cerr << "Failed to create `" << destParentAbsolutePath << "': " << rs.message() << '\n';
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    std::ifstream inFile(sourceAbsolutePath);
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

    std::ofstream outFile(destAbsolutePath);
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

    std::cout << "Minified `" << sourceAbsolutePath << "' to `" << destAbsolutePath << "'\n";

    return true;
}
