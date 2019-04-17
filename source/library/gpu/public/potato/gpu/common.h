// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/span.h"
#include "potato/foundation/int_types.h"

namespace up {
    enum class GpuFormat {
        Unknown,
        R32G32B32A32Float,
        R32G32B32Float,
        R32G32Float,
        R8G8B8A8UnsignedNormalized,
        D32Float
    };

    enum class GpuShaderSemantic {
        Position,
        Color,
        TexCoord
    };

    struct GpuInputLayoutElement {
        GpuFormat format = GpuFormat::Unknown;
        GpuShaderSemantic semantic = GpuShaderSemantic::Position;
        uint32 semanticIndex = 0;
        uint32 slot = 0;
    };

    enum class GpuViewType {
        RTV,
        UAV,
        SRV,
        DSV
    };

    enum class GpuBufferType {
        Constant,
        Index,
        Vertex,
    };

    enum class GpuIndexFormat {
        Unsigned16,
        Unsigned32
    };

    enum class GpuShaderStage {
        Vertex = 1 << 0,
        Pixel = 1 << 1,
        All = Vertex | Pixel
    };

    enum class GpuTextureType {
        Texture2D,
        Texture3D,
        DepthStencil,
    };

    enum class GpuPrimitiveTopology {
        Triangles,
        Lines,
    };

    struct GpuClipRect {
        uint32 left = 0, top = 0, right = 0, bottom = 0;
    };

    struct GpuViewportDesc {
        float leftX = 0, topY = 1;
        float width = 0, height = 0;
        float minDepth = 0, maxDepth = 1;
    };

    struct GpuDeviceInfo {
        int index;
    };

    struct GpuTextureDesc {
        GpuFormat format = GpuFormat::Unknown;
        GpuTextureType type = GpuTextureType::DepthStencil;
        uint32 width = 0, height = 0;
    };

    struct GpuPipelineStateDesc {
        bool enableScissor = false;
        bool enableDepthWrite = false;
        bool enableDepthTest = false;
        span<byte const> vertShader;
        span<byte const> pixelShader;
        span<GpuInputLayoutElement const> inputLayout;
    };

} // namespace up
