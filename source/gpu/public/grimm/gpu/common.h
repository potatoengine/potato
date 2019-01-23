// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/blob.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/types.h"

namespace gm::gpu {
    enum class Format {
        Unknown,
        R32G32B32A32Float,
        R32G32B32Float,
        R32G32Float,
        R8G8B8A8UnsignedNormalized,
    };

    enum class Semantic {
        Position,
        Color,
        TexCoord
    };

    struct InputLayoutElement {
        Format format = Format::Unknown;
        Semantic semantic = Semantic::Position;
        uint32 semanticIndex = 0;
        uint32 slot = 0;
    };

    enum class ViewType {
        RTV,
        UAV,
        SRV,
        DSV
    };

    enum class BufferType {
        Constant,
        Index,
        Vertex,
    };

    enum class IndexType {
        Unsigned16,
        Unsigned32
    };

    enum class ShaderStage {
        Vertex = 1 << 0,
        Pixel = 1 << 1,
        All = Vertex | Pixel
    };

    enum class TextureType {
        Texture2D,
        Texture3D,
        DepthStencil,
    };

    enum class PrimitiveTopology {
        Triangles,
    };

    struct Viewport {
        float width = 800, height = 600;
        float minDepth = 0, maxDepth = 1;
        float leftX = 0, topY = 1;
    };

    struct DeviceInfo {
        int index;
    };

    struct PipelineStateDesc {
        blob vertShader;
        blob pixelShader;
        span<InputLayoutElement const> inputLayout;
    };

} // namespace gm::gpu
