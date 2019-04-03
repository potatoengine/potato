// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"

namespace up::gpu {
    class ResourceView {
    public:
        ResourceView() = default;
        virtual ~ResourceView() = default;

        ResourceView(ResourceView&&) = delete;
        ResourceView& operator=(ResourceView&&) = delete;

        virtual ViewType type() const = 0;
    };
} // namespace up::gpu
