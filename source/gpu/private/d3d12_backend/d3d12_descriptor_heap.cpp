// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_descriptor_heap.h"
#    include "grimm/foundation/out_ptr.h"

gm::D3d12DescriptorHeap::D3d12DescriptorHeap(com_ptr<ID3D12DescriptorHeap> heap, int descriptorSize) : _heap(std::move(heap)), _descriptorSize(descriptorSize) {}

gm::D3d12DescriptorHeap::~D3d12DescriptorHeap() = default;

auto gm::D3d12DescriptorHeap::createDescriptorHeap(ID3D12Device1* device) -> box<IDescriptorHeap> {
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 1;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    com_ptr<ID3D12DescriptorHeap> heap;
    device->CreateDescriptorHeap(&heapDesc, __uuidof(ID3D12DescriptorHeap), out_ptr(heap));
    if (heap == nullptr) {
        return nullptr;
    }

    int descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    return make_box<D3d12DescriptorHeap>(std::move(heap), descriptorSize);
}

#endif
