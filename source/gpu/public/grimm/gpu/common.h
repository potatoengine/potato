// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
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

    enum class PrimitiveTopology {
        Triangles,
    };

    struct Viewport {
        float width = 800, height = 600;
        float minDepth = 0, maxDepth = 1;
        float leftX = 0, topY = 1;
    };
} // namespace gm
