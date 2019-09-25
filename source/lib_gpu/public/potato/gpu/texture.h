// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include <glm/vec3.hpp>

namespace up {
    class GpuTexture {
    public:
        GpuTexture() = default;
        virtual ~GpuTexture() = default;

        GpuTexture(GpuTexture&&) = delete;
        GpuTexture& operator=(GpuTexture&&) = delete;

        virtual GpuTextureType type() const noexcept = 0;
        virtual GpuFormat format() const noexcept = 0;
        virtual glm::ivec3 dimensions() const noexcept = 0;
    };
} // namespace up