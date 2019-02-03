// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "renderer.h"
#include "render_task.h"
#include "context.h"
#include <grimm/gpu/buffer.h>
#include <grimm/gpu/command_list.h>
#include <grimm/gpu/device.h>
#include <grimm/gpu/swap_chain.h>
#include <chrono>

namespace {
    struct FrameData {
        gm::uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };
} // namespace

gm::Renderer::Renderer(rc<gpu::Device> device) : _device(std::move(device)), _renderThread([this] { _renderMain(); }) {
    _commandList = _device->createCommandList();
}

gm::Renderer::~Renderer() {
    _taskQueue.close();
    _renderThread.join();
}

void gm::Renderer::_renderMain() {
    RenderTask task;
    while (_taskQueue.dequeWait(task)) {
        task();
    }
}

void gm::Renderer::beginFrame() {
    if (_frameDataBuffer == nullptr) {
        _frameDataBuffer = _device->createBuffer(gpu::BufferType::Constant, sizeof(FrameData));
    }

    uint64 nowNanoseconds = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (_startTimestamp == 0) {
        _startTimestamp = nowNanoseconds;
    }

    double const now = static_cast<double>(nowNanoseconds - _startTimestamp) / 1000000000.0;
    FrameData frame = {
        _frameCounter++,
        static_cast<float>(now - _frameTimestamp),
        now};
    _frameTimestamp = now;

    _commandList->clear();
    _commandList->update(_frameDataBuffer.get(), view<byte>{reinterpret_cast<byte*>(&frame), sizeof(frame)});
    _commandList->bindConstantBuffer(0, _frameDataBuffer.get(), gpu::ShaderStage::All);
}

void gm::Renderer::endFrame() {
    _commandList->finish();
    _device->execute(_commandList.get());
}

auto gm::Renderer::context() -> RenderContext {
    return RenderContext{
        _frameTimestamp,
        *_commandList,
        *_device};
}
