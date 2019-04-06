// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace up::gpu {
    class Sampler {
    public:
        Sampler() = default;
        virtual ~Sampler() = default;

        Sampler(Sampler&&) = delete;
        Sampler& operator=(Sampler&&) = delete;
    };
} // namespace up::gpu