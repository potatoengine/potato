// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "camera.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/texture.h"
#include "grimm/gpu/resource_view.h"

gm::Camera::Camera(rc<gpu::SwapChain> swapChain) : _swapChain(std::move(swapChain)) {}

gm::Camera::~Camera() = default;

void gm::Camera::resetSwapChain(rc<gpu::SwapChain> swapChain) {
    _swapChain = std::move(swapChain);
    _backBuffer.reset();
    _rtv.reset();
}

void gm::Camera::beginFrame(gpu::CommandList& commandList, gpu::Device& device) {
    if (_rtv == nullptr && _swapChain != nullptr) {
        _backBuffer = _swapChain->getBuffer(0);
        _rtv = device.createRenderTargetView(_backBuffer.get());
    }

    gpu::Viewport viewport;
    auto dimensions = _backBuffer->dimensions();
    viewport.width = dimensions.m.x;
    viewport.height = dimensions.m.y;

    commandList.clearRenderTarget(_rtv.get(), {0.f, 0.f, 0.1f, 1.f});
    commandList.bindRenderTarget(0, _rtv.get());
    commandList.setViewport(viewport);
}

void gm::Camera::endFrame(gpu::CommandList& commandList, gpu::Device& device) {
}
