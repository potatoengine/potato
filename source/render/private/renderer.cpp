// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "renderer.h"
#include "render_task.h"
#include <grimm/gpu/device.h>
#include <grimm/gpu/swap_chain.h>

gm::Renderer::Renderer(rc<gpu::Device> device) : _device(std::move(device)), _renderThread([this] { _renderMain(); }) {}

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
