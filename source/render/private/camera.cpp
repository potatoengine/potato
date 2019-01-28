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
    struct alignas(16) CameraData {
        float worldViewProjection[16];
        float worldView[16];
        float viewProjection[16];
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

    auto dimensions = _backBuffer->dimensions();

    gpu::Viewport viewport;
    viewport.width = dimensions.m.x;
    viewport.height = dimensions.m.y;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    float farZ = 4000.f;
    float nearZ = 2.f;
    float aspect = viewport.width / viewport.height;
    float fovY = constants::degreesToRadians<float> * 75.f;

    CameraData data;
    Matrix4f worldView;
    worldView *= rotationXY(static_cast<float>(std::fmod(ctx.frameTime, 2.0 * constants::pi<double>)));
    worldView.alignedStore(data.worldView);

    Matrix4f viewProjection = projection(aspect, fovY, nearZ, farZ);
    viewProjection.alignedStore(data.viewProjection);

    auto modelViewProjection = worldView * viewProjection;
    modelViewProjection.alignedStore(data.worldViewProjection);

    ctx.commandList.update(_cameraDataBuffer.get(), span{&data, 1}.as_bytes());

    ctx.commandList.clearRenderTarget(_rtv.get(), {0.f, 0.f, 0.1f, 1.f});
    ctx.commandList.bindRenderTarget(0, _rtv.get());
    ctx.commandList.bindConstantBuffer(1, _cameraDataBuffer.get(), gpu::ShaderStage::All);
    ctx.commandList.setViewport(viewport);
}

void gm::Camera::endFrame(RenderContext& ctx) {
}
