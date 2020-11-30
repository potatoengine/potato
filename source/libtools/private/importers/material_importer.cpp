// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importers/material_importer.h"

#include "potato/render/material_generated.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/path.h"
#include "potato/spud/std_iostream.h"

#include <nlohmann/json.hpp>
#include <fstream>

up::MaterialImporter::MaterialImporter() = default;

up::MaterialImporter::~MaterialImporter() = default;

bool up::MaterialImporter::import(ImporterContext& ctx) {
    auto sourceAbsolutePath = path::join(ctx.sourceFolderPath(), ctx.sourceFilePath());
    auto destAbsolutePath = path::join(ctx.destinationFolderPath(), ctx.sourceFilePath());

    string destParentAbsolutePath(path::parent(destAbsolutePath));

    if (!fs::directoryExists(destParentAbsolutePath.c_str())) {
        if (fs::createDirectories(destParentAbsolutePath.c_str()) != IOResult::Success) {
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

    auto jsonShaders = doc["shaders"];
    if (!jsonShaders.is_object()) {
        return false;
    }

    auto vertexUuid = UUID::fromString(jsonShaders["vertex"].get<string_view>());
    auto pixelUuid = UUID::fromString(jsonShaders["pixel"].get<string_view>());

    flatbuffers::FlatBufferBuilder builder;

    schema::UUID schemaVertexUuid;
    schema::UUID schemaPixelUuid;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    std::memcpy(const_cast<int8_t*>(schemaVertexUuid.b()->data()), vertexUuid.bytes(), UUID::octects);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    std::memcpy(const_cast<int8_t*>(schemaPixelUuid.b()->data()), pixelUuid.bytes(), UUID::octects);

    std::vector<schema::UUID> textures;

    auto jsonTextures = doc["textures"];
    for (auto const& jsonTexture : jsonTextures) {
        auto textureUuid = UUID::fromString(jsonTexture.get<string_view>());
        schema::UUID& schemaTextureUuid = textures.emplace_back();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        std::memcpy(const_cast<int8_t*>(schemaTextureUuid.b()->data()), textureUuid.bytes(), UUID::octects);
    }

    auto mat = schema::CreateMaterialDirect(
        builder,
        schema::CreateMaterialShader(builder, &schemaVertexUuid, &schemaPixelUuid),
        &textures);

    builder.Finish(mat);

    std::ofstream outFile(destAbsolutePath.c_str());
    if (!outFile) {
        ctx.logger().error("Failed to open `{}'", destAbsolutePath);
        return false;
    }

    outFile.write(reinterpret_cast<char const*>(builder.GetBufferPointer()), builder.GetSize());

    outFile.close();

    // output has same name as input
    ctx.addMainOutput(string{ctx.sourceFilePath()}, "potato.asset.material");

    ctx.logger().info("Compiled `{}' to `{}'", sourceAbsolutePath, destAbsolutePath);

    return true;
}
