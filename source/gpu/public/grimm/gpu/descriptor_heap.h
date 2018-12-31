// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/types.h"

namespace gm {
    struct GpuDescriptorHandle {
        uint64 ptr;
        uint64 size;
    };

    class GpuDescriptorHeap {
    public:
        GpuDescriptorHeap() = default;
        virtual ~GpuDescriptorHeap() = default;

        GpuDescriptorHeap(GpuDescriptorHeap&&) = delete;
        GpuDescriptorHeap& operator=(GpuDescriptorHeap&&) = delete;

        virtual GpuDescriptorHandle getCpuHandle() const = 0;
    };
} // namespace gm
