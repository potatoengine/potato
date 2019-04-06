// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "d3d11_platform.h"
#include "potato/foundation/assertion.h"

auto up::gpu::d3d11::toNative(Semantic semantic) noexcept -> zstring_view {
    switch (semantic) {
    case Semantic::Position: return "POSITION";
    case Semantic::Color: return "COLOR";
    case Semantic::TexCoord: return "TEXCOORD";
    default: UP_UNREACHABLE("Unknown Semantic"); return "UNKNOWN";
    }
}

auto up::gpu::d3d11::toNative(Format format) noexcept -> DXGI_FORMAT {
    switch (format) {
    case Format::R32G32B32A32Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case Format::R32G32B32Float: return DXGI_FORMAT_R32G32B32_FLOAT;
    case Format::R32G32Float: return DXGI_FORMAT_R32G32_FLOAT;
    case Format::R8G8B8A8UnsignedNormalized: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Format::D32Float: return DXGI_FORMAT_D32_FLOAT;
    default: UP_UNREACHABLE("Unknown Format"); return DXGI_FORMAT_UNKNOWN;
    }
}

auto up::gpu::d3d11::fromNative(DXGI_FORMAT format) noexcept -> Format {
    switch (format) {
    case DXGI_FORMAT_R32G32B32A32_FLOAT: return Format::R32G32B32A32Float;
    case DXGI_FORMAT_R32G32B32_FLOAT: return Format::R32G32B32Float;
    case DXGI_FORMAT_R32G32_FLOAT: return Format::R32G32Float;
    case DXGI_FORMAT_R8G8B8A8_UNORM: return Format::R8G8B8A8UnsignedNormalized;
    case DXGI_FORMAT_D32_FLOAT: return Format::D32Float;
    default: return Format::Unknown;
    }
}

auto up::gpu::d3d11::toByteSize(Format format) noexcept -> up::uint32 {
    switch (format) {
    case Format::R32G32B32A32Float:
        return 16;
    case Format::R32G32B32Float:
        return 12;
    case Format::R32G32Float:
        return 8;
    case Format::R8G8B8A8UnsignedNormalized:
    case Format::D32Float:
        return 4;
    default: UP_UNREACHABLE("Unknown Format"); return 0;
    }
}

auto up::gpu::d3d11::toNative(IndexType type) noexcept -> DXGI_FORMAT {
    switch (type) {
    case IndexType::Unsigned16: return DXGI_FORMAT_R16_UINT;
    case IndexType::Unsigned32: return DXGI_FORMAT_R32_UINT;
    default: UP_UNREACHABLE("Unknown IndexType"); return DXGI_FORMAT_UNKNOWN;
    }
}
