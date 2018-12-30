// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_device.h"
#include "command_list.h"
#include "descriptor_heap.h"
#include "resource.h"
#include "swap_chain.h"

gm::NullDevice::NullDevice() = default;
gm::NullDevice::~NullDevice() = default;

auto gm::NullDevice::createSwapChain(void* native_window) -> box<ISwapChain> {
    return nullptr;
}

auto gm::NullDevice::createDescriptorHeap() -> box<IDescriptorHeap> {
    return nullptr;
}

auto gm::NullDevice::createCommandList() -> box<ICommandList> {
    return nullptr;
}

void gm::NullDevice::createRenderTargetView(IGpuResource* renderTarget, uint64 cpuHandle) {}

void gm::NullDevice::execute(ICommandList* commands) {}
