// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/gpu_resource_view.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
    struct alignas(16) CameraData {
        glm::mat4x4 worldViewProjection;
        glm::mat4x4 worldView;
        glm::mat4x4 viewProjection;
        glm::vec3 cameraPosition;
        glm::vec2 nearFar;
    };
} // namespace

up::RenderCamera::RenderCamera() = default;

up::RenderCamera::~RenderCamera() = default;

void up::RenderCamera::resetBackBuffer(box<GpuTexture> texture) {
    _backBuffer = std::move(texture);
    _depthStencilBuffer.reset();
    _rtv.reset();
    _dsv.reset();
}

void up::RenderCamera::beginFrame(RenderContext& ctx, glm::vec3 cameraPosition, glm::mat4x4 cameraTransform) {
    if (_rtv == nullptr && _backBuffer != nullptr) {
        _rtv = ctx.device.createRenderTargetView(_backBuffer.get());
    }

    auto dimensions = _backBuffer->dimensions();

    if (_dsv == nullptr) {
        GpuTextureDesc desc;
        desc.type = GpuTextureType::DepthStencil;
        desc.format = GpuFormat::D32Float;
        desc.width = static_cast<uint32>(dimensions.x);
        desc.height = static_cast<uint32>(dimensions.y);
        _depthStencilBuffer = ctx.device.createTexture2D(desc, {});
        _dsv = ctx.device.createDepthStencilView(_depthStencilBuffer.get());
    }

    if (_cameraDataBuffer == nullptr) {
        _cameraDataBuffer = ctx.device.createBuffer(GpuBufferType::Constant, sizeof(CameraData));
    }

    GpuViewportDesc viewport;
    viewport.width = static_cast<float>(dimensions.x);
    viewport.height = static_cast<float>(dimensions.y);
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    float farZ = 400.f;
    float nearZ = .2f;

    auto projection = glm::perspectiveFovRH_ZO(glm::radians(75.f), viewport.width, viewport.height, nearZ, farZ);

    CameraData data;
    data.worldView = transpose(cameraTransform);
    data.viewProjection = transpose(projection);
    data.worldViewProjection = cameraTransform * projection;
    data.cameraPosition = cameraPosition;
    data.nearFar = {nearZ, farZ};

    ctx.commandList.update(_cameraDataBuffer.get(), span{&data, 1}.as_bytes());

    ctx.commandList.clearRenderTarget(_rtv.get(), {0.f, 0.f, 0.1f, 1.f});
    ctx.commandList.clearDepthStencil(_dsv.get());
    ctx.commandList.bindRenderTarget(0, _rtv.get());
    ctx.commandList.bindDepthStencil(_dsv.get());
    ctx.commandList.bindConstantBuffer(1, _cameraDataBuffer.get(), GpuShaderStage::All);
    ctx.commandList.setViewport(viewport);
}

void up::RenderCamera::endFrame(RenderContext& ctx) {
}
