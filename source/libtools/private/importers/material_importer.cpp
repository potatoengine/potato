// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "importers/material_importer.h"
#include "material_schema.h"

#include "potato/reflex/serialize.h"
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

    schema::Material material;
    if (!reflex::decodeFromJson(doc, material)) {
        ctx.logger().error("Failed to deserialize `{}': {}", sourceAbsolutePath);
        return false;
    }

    flatbuffers::FlatBufferBuilder builder;

    flat::UUID schemaVertexUuid;
    flat::UUID schemaPixelUuid;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    std::memcpy(const_cast<int8_t*>(schemaVertexUuid.b()->data()), material.shaders.vertex.bytes(), UUID::octects);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    std::memcpy(const_cast<int8_t*>(schemaPixelUuid.b()->data()), material.shaders.pixel.bytes(), UUID::octects);

    std::vector<flat::UUID> textures;

    auto jsonTextures = doc["textures"];
    for (UUID const& textureUuid : material.textures) {
        flat::UUID& schemaTextureUuid = textures.emplace_back();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        std::memcpy(const_cast<int8_t*>(schemaTextureUuid.b()->data()), textureUuid.bytes(), UUID::octects);
    }

    auto mat = flat::CreateMaterialDirect(
        builder,
        flat::CreateMaterialShader(builder, &schemaVertexUuid, &schemaPixelUuid),
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
