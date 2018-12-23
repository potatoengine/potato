// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/types.h"

namespace gm {
    class IDescriptorHeap {
    public:
        IDescriptorHeap() = default;
        virtual ~IDescriptorHeap();

        IDescriptorHeap(IDescriptorHeap&&) = delete;
        IDescriptorHeap& operator=(IDescriptorHeap&&) = delete;

        virtual uint64 getCpuHandle() const = 0;
        virtual uint64 getCpuHandleSize() const = 0;
    };
} // namespace gm
