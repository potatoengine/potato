// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "camera.h"
#include "context.h"
#include "gpu_buffer.h"
#include "gpu_command_list.h"
#include "gpu_device.h"
#include "gpu_resource_view.h"
#include "gpu_texture.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "potato/runtime/assertion.h"

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

void up::RenderCamera::setRenderTarget(box<GpuResourceView> rtv) {
    _rtv = std::move(rtv);
}

void up::RenderCamera::updateBuffers(
    RenderContext& ctx,
    glm::vec3 dimensions,
    glm::vec3 cameraPosition,
    glm::mat4x4 cameraTransform) {

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

    constexpr float farZ = 400.f;
    constexpr float nearZ = .2f;
    constexpr float fovDeg = 75.f;

    auto projection = glm::perspectiveFovRH_ZO(glm::radians(fovDeg), viewport.width, viewport.height, nearZ, farZ);

    auto data = CameraData{
        .worldViewProjection = cameraTransform * projection,
        .worldView = transpose(cameraTransform),
        .viewProjection = transpose(projection),
        .cameraPosition = cameraPosition,
        .nearFar = {nearZ, farZ},
    };

    ctx.commandList.update(_cameraDataBuffer.get(), span{&data, 1}.as_bytes());
}

void up::RenderCamera::beginFrame(RenderContext& ctx, glm::vec3 cameraPosition, glm::mat4x4 cameraTransform) {
    auto dimensions = glm::vec3(1024.0f, 768.0f, 0.0f);
    
    GpuViewportDesc viewport;
    viewport.width = static_cast<float>(dimensions.x);
    viewport.height = static_cast<float>(dimensions.y);
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    constexpr float farZ = 400.f;
    constexpr float nearZ = .2f;
    constexpr float fovDeg = 75.f;

    updateBuffers(ctx, dimensions, cameraPosition, cameraTransform);

    constexpr glm::vec4 clearColor{0.f, 0.f, 0.1f, 1.f};

    ctx.commandList.clearRenderTarget(_rtv.get(), clearColor);
    ctx.commandList.clearDepthStencil(_dsv.get());
    ctx.commandList.bindRenderTarget(0, _rtv.get());
    ctx.commandList.bindDepthStencil(_dsv.get());
    //ctx.commandList.bindConstantBuffer(1, _cameraDataBuffer.get(), GpuShaderStage::All);
    ctx.commandList.setViewport(viewport);
}
