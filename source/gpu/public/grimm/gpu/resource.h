// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm::gpu {
    class Resource {
    public:
        Resource() = default;
        virtual ~Resource() = default;

        Resource(Resource&&) = delete;
        Resource& operator=(Resource&&) = delete;
    };
} // namespace gm::gpu
