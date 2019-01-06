// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_copy.h"
#include <iostream>

gm::recon::CopyConverter::CopyConverter() = default;

gm::recon::CopyConverter::~CopyConverter() = default;

bool gm::recon::CopyConverter::convert(Context& ctx) {

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

    if (std::filesystem::copy_file(sourceAbsolutePath, destParentAbsolutePath, rs)) {
        std::cerr << "Failed to coy `" << sourceAbsolutePath << "' to `" << destAbsolutePath << "': " << rs.message() << '\n';
        return false;
    }

    std::cout << "Copied `" << sourceAbsolutePath << "' to `" << destAbsolutePath << "'\n";

    return true;
}
