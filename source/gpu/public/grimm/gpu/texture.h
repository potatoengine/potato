// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm::gpu {
    class Texture {
    public:
        Texture() = default;
        virtual ~Texture() = default;

        Texture(Texture&&) = delete;
        Texture& operator=(Texture&&) = delete;
    };
} // namespace gm::gpu
