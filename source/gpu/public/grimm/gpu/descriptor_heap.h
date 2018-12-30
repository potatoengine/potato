// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/types.h"

namespace gm {
    struct DescriptorHandle {
        uint64 ptr;
        uint64 size;
    };

    class IDescriptorHeap {
    public:
        IDescriptorHeap() = default;
        virtual ~IDescriptorHeap() = default;

        IDescriptorHeap(IDescriptorHeap&&) = delete;
        IDescriptorHeap& operator=(IDescriptorHeap&&) = delete;

        virtual DescriptorHandle getCpuHandle() const = 0;
    };
} // namespace gm
