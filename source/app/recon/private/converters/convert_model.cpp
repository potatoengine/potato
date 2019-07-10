// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "convert_model.h"
#include "potato/foundation/std_iostream.h"
#include "potato/filesystem/path.h"
#include "potato/filesystem/filesystem.h"
#include "potato/filesystem/stream.h"
#include "potato/logger/logger.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>

up::recon::ModelConverter::ModelConverter() = default;

up::recon::ModelConverter::~ModelConverter() = default;

bool up::recon::ModelConverter::convert(Context& ctx) {
    auto sourceAbsolutePath = path::join({ctx.sourceFolderPath(), ctx.sourceFilePath()});
    auto destAbsolutePath = path::join({ctx.destinationFolderPath(), path::changeExtension(ctx.sourceFilePath(), ".model")});

    string destParentAbsolutePath(path::parent(string_view(destAbsolutePath)));

    if (!ctx.fileSystem().directoryExists(destParentAbsolutePath.c_str())) {
        if (ctx.fileSystem().createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
            ctx.logger().error("Failed to create `{}'", destParentAbsolutePath);
            // intentionally fall through so we still attempt the copy and get a copy error if fail
        }
    }

    auto file = ctx.fileSystem().openRead(sourceAbsolutePath);

    vector<byte> contents;
    if (readBinary(file, contents) != IOResult::Success) {
        ctx.logger().error("Failed to read");
        return false;
    }

    file.close();

    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFileFromMemory(contents.data(), contents.size(), aiProcess_FlipWindingOrder | aiProcess_CalcTangentSpace | aiProcessPreset_TargetRealtime_Fast);
    if (scene == nullptr) {
        ctx.logger().error("Failed to decode");
        return false;
    }

    if (scene->mNumMeshes != 1) {
        ctx.logger().error("Only single meshes are supported");
        return false;
    }

    Assimp::Exporter exporter;
    aiExportDataBlob const* data = exporter.ExportToBlob(scene, "assbin", 0);

    file = ctx.fileSystem().openWrite(destAbsolutePath);
    file.write(span{reinterpret_cast<byte const*>(data->data), data->size});
    file.close();

    ctx.logger().info("Wrote optimized model to `{}'", destAbsolutePath);

    return true;
}
