// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/render/camera.h"
#include "grimm/render/context.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/texture.h"
#include "grimm/gpu/resource_view.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
    struct alignas(16) CameraData {
        glm::mat4x4 worldViewProjection;
        glm::mat4x4 worldView;
        glm::mat4x4 viewProjection;
    };
} // namespace

gm::RenderCamera::RenderCamera(rc<gpu::SwapChain> swapChain) : _swapChain(std::move(swapChain)) {}

gm::RenderCamera::~RenderCamera() = default;

void gm::RenderCamera::resetSwapChain(rc<gpu::SwapChain> swapChain) {
    _swapChain = std::move(swapChain);
    _backBuffer.reset();
    _depthStencilBuffer.reset();
    _rtv.reset();
    _dsv.reset();
}

void gm::RenderCamera::beginFrame(RenderContext& ctx, glm::mat4x4 cameraTransform) {
    if (_rtv == nullptr && _swapChain != nullptr) {
        _backBuffer = _swapChain->getBuffer(0);
        _rtv = ctx.device.createRenderTargetView(_backBuffer.get());
    }

    auto dimensions = _backBuffer->dimensions();

    if (_dsv == nullptr) {
        gpu::TextureDesc desc;
        desc.type = gpu::TextureType::DepthStencil;
        desc.format = gpu::Format::D32Float;
        desc.width = static_cast<uint32>(dimensions.x);
        desc.height = static_cast<uint32>(dimensions.y);
        _depthStencilBuffer = ctx.device.createTexture2D(desc, {});
        _dsv = ctx.device.createDepthStencilView(_depthStencilBuffer.get());
    }

    if (_cameraDataBuffer == nullptr) {
        _cameraDataBuffer = ctx.device.createBuffer(gpu::BufferType::Constant, sizeof(CameraData));
    }

    gpu::Viewport viewport;
    viewport.width = static_cast<float>(dimensions.x);
    viewport.height = static_cast<float>(dimensions.y);
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    float farZ = 4000.f;
    float nearZ = 2.f;

    auto projection = glm::perspectiveFovRH_ZO(glm::radians(75.f), viewport.width, viewport.height, nearZ, farZ);

    CameraData data;
    data.worldView = transpose(cameraTransform);
    data.viewProjection = transpose(projection);
    data.worldViewProjection = cameraTransform * projection;

    ctx.commandList.update(_cameraDataBuffer.get(), span{&data, 1}.as_bytes());

    ctx.commandList.clearRenderTarget(_rtv.get(), {0.f, 0.f, 0.1f, 1.f});
    ctx.commandList.clearDepthStencil(_dsv.get());
    ctx.commandList.bindRenderTarget(0, _rtv.get());
    ctx.commandList.bindDepthStencil(_dsv.get());
    ctx.commandList.bindConstantBuffer(1, _cameraDataBuffer.get(), gpu::ShaderStage::All);
    ctx.commandList.setViewport(viewport);
}

void gm::RenderCamera::endFrame(RenderContext& ctx) {
}