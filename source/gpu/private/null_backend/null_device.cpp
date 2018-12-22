// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "null_device.h"
#include "swap_chain.h"

gm::NullDevice::NullDevice() = default;
gm::NullDevice::~NullDevice() = default;

auto gm::NullDevice::createSwapChain(void* native_window) -> box<ISwapChain> {
    return nullptr;
}
