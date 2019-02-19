// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_model.h"
#include "grimm/foundation/std_iostream.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/stream_util.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>

gm::recon::ModelConverter::ModelConverter() = default;

gm::recon::ModelConverter::~ModelConverter() = default;

bool gm::recon::ModelConverter::convert(Context& ctx) {
    auto sourceAbsolutePath = fs::path::join({ctx.sourceFolderPath(), ctx.sourceFilePath()});
    auto destAbsolutePath = fs::path::join({ctx.destinationFolderPath(), fs::path::changeExtension(ctx.sourceFilePath(), ".model")});

    string destParentAbsolutePath(fs::path::parent(string_view(destAbsolutePath)));

    fs::FileSystem fileSys;

    if (!fileSys.directoryExists(destParentAbsolutePath.c_str())) {
        if (fileSys.createDirectories(destParentAbsolutePath.c_str()) != fs::Result::Success) {
            std::cerr << "Failed to create `" << destParentAbsolutePath << '\n';
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    auto file = fileSys.openRead(sourceAbsolutePath);

    vector<byte> contents;
    if (readBinary(file, contents) != fs::Result::Success) {
        std::cerr << "Failed to read\n";
        return false;
    }

    file.close();

    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFileFromMemory(contents.data(), contents.size(), aiProcess_FlipWindingOrder | aiProcessPreset_TargetRealtime_Fast);
    if (scene == nullptr) {
        std::cerr << "Failed to decode\n";
        return false;
    }

    if (scene->mNumMeshes != 1) {
        std::cerr << "Only single meshes are supported\n";
        return false;
    }

    Assimp::Exporter exporter;
    aiExportDataBlob const* data = exporter.ExportToBlob(scene, "assbin", 0);

    file = fileSys.openWrite(destAbsolutePath);
    file.write(span{reinterpret_cast<byte const*>(data->data), data->size});
    file.close();

    std::cout << "Wrote optimized model to `" << destAbsolutePath << "'\n";

    return true;
}
