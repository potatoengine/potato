// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "camera.h"
#include "context.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/texture.h"
#include "grimm/gpu/resource_view.h"
#include "grimm/math/packed.h"
#include "grimm/math/vector.h"
#include "grimm/math/geometry.h"
#include "grimm/math/constants.h"

namespace {
    struct CameraData alignas(16) {
        float modelView[16];
        gm::PackedVector4f viewProjection[4];
    };
} // namespace

gm::Camera::Camera(rc<gpu::SwapChain> swapChain) : _swapChain(std::move(swapChain)) {}

gm::Camera::~Camera() = default;

void gm::Camera::resetSwapChain(rc<gpu::SwapChain> swapChain) {
    _swapChain = std::move(swapChain);
    _backBuffer.reset();
    _rtv.reset();
}

void gm::Camera::beginFrame(RenderContext& ctx) {
    if (_rtv == nullptr && _swapChain != nullptr) {
        _backBuffer = _swapChain->getBuffer(0);
        _rtv = ctx.device.createRenderTargetView(_backBuffer.get());
    }

    if (_cameraDataBuffer == nullptr) {
        _cameraDataBuffer = ctx.device.createBuffer(gpu::BufferType::Constant, sizeof(CameraData));
    }

    gpu::Viewport viewport;
    auto dimensions = _backBuffer->dimensions();
    viewport.width = dimensions.m.x;
    viewport.height = dimensions.m.y;

    float farZ = 1.f;
    float nearZ = 0.f;
    float a = 1.f / (farZ - nearZ);
    float b = a * nearZ;

    CameraData data;
    Matrix4f modelView;
    modelView = modelView * rotationXY(static_cast<float>(std::fmod(ctx.frameTime, 2.0 * constants::pi<double>)));
    modelView.alignedStore(data.modelView);

    data.viewProjection[0] = {.5f * viewport.width, 0, 0, 0};
    data.viewProjection[1] = {0, .5f * viewport.height, 0, 0};
    data.viewProjection[2] = {0, 0, a, 0};
    data.viewProjection[3] = {0, 0, b, 1};

    ctx.commandList.update(_cameraDataBuffer.get(), span{&data, 1}.as_bytes());

    ctx.commandList.clearRenderTarget(_rtv.get(), {0.f, 0.f, 0.1f, 1.f});
    ctx.commandList.bindRenderTarget(0, _rtv.get());
    ctx.commandList.bindConstantBuffer(1, _cameraDataBuffer.get(), gpu::ShaderStage::All);
    ctx.commandList.setViewport(viewport);
}

void gm::Camera::endFrame(RenderContext& ctx) {
}
