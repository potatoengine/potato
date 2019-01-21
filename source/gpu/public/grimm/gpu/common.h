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
} // namespace gm
