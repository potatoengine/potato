// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/math/packed.h"

namespace gm::gpu {
    class Texture {
    public:
        Texture() = default;
        virtual ~Texture() = default;

        Texture(Texture&&) = delete;
        Texture& operator=(Texture&&) = delete;

        virtual TextureType type() const noexcept = 0;
        virtual Format format() const noexcept = 0;
        virtual Packed3 dimensions() const noexcept = 0;
    };
} // namespace gm::gpu
