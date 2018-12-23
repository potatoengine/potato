// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
    class IDescriptorHeap {
    public:
        IDescriptorHeap() = default;
        virtual ~IDescriptorHeap();

        IDescriptorHeap(IDescriptorHeap&&) = delete;
        IDescriptorHeap& operator=(IDescriptorHeap&&) = delete;
    };
} // namespace gm
