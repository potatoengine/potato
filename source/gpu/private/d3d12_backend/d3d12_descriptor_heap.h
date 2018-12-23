// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/descriptor_heap.h"

namespace gm {
    class D3d12DescriptorHeap final : public IDescriptorHeap {
    public:
        D3d12DescriptorHeap(com_ptr<ID3D12DescriptorHeap> heap, int descriptorSize);
        virtual ~D3d12DescriptorHeap();

        D3d12DescriptorHeap(D3d12DescriptorHeap&&) = delete;
        D3d12DescriptorHeap& operator=(D3d12DescriptorHeap&&) = delete;

        static box<IDescriptorHeap> createDescriptorHeap(ID3D12Device1* device);

    private:
        com_ptr<ID3D12DescriptorHeap> _heap;
        int _descriptorSize = -1;
    };
} // namespace gm
