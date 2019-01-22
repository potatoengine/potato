// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "d3d11_platform.h"

auto gm::gpu::d3d11::toNative(Semantic semantic) noexcept -> zstring_view {
    switch (semantic) {
    case Semantic::Position: return "POSITION";
    case Semantic::Color: return "COLOR";
    default: return "UNKNOWN";
    }
}

auto gm::gpu::d3d11::toNative(Format format) noexcept -> DXGI_FORMAT {
    switch (format) {
    case Format::R32G32B32A32Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case Format::R32G32B32Float: return DXGI_FORMAT_R32G32B32_FLOAT;
    case Format::R8G8B8A8UnsignedNormalized: return DXGI_FORMAT_R8G8B8A8_UNORM;
    default: return DXGI_FORMAT_UNKNOWN;
    }
}
