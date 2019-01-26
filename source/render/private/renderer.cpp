// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "renderer.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/swap_chain.h"

struct gm::Renderer::Backend {
    Backend(rc<gpu::Device> device);
    ~Backend();

    void renderMain();
    void shutdown();

    rc<gpu::Device> device;
    std::thread renderThread;
    std::atomic<bool> running = true;
};

gm::Renderer::Backend::Backend(rc<gpu::Device> device) : device(std::move(device)), renderThread([this] { renderMain(); }) {
}

gm::Renderer::Backend::~Backend() {
    running = true;
    renderThread.join();
}

void gm::Renderer::Backend::renderMain() {
    while (running) {
    }
}

gm::Renderer::Renderer(rc<gpu::Device> device) : _backend(make_box<Backend>(std::move(device))) {}

gm::Renderer::~Renderer() = default;

void gm::Renderer::bindSwapChain(rc<gpu::SwapChain> swapChain) {
}
