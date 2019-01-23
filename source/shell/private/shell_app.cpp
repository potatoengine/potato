// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "shell_app.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/vector.h"
#include "grimm/filesystem/stream.h"
#include "grimm/filesystem/stream_util.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/factory.h"
#include "grimm/gpu/swap_chain.h"
#include "grimm/gpu/texture.h"
#include "grimm/math/packed.h"
#include "grimm/grui/grui.h"

#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <imgui.h>

static constexpr gm::PackedVector3f triangle[] = {
    {-0.8f, -0.8f, 0},
    {1, 0, 0},
    {0.8f, -0.8f, 0},
    {0, 1, 0},
    {0, +0.8f, 0},
    {0, 0, 1},
};

gm::ShellApp::~ShellApp() {
    _drawImgui.releaseResources();

    _commandList.reset();
    _rtv.reset();
    _pipelineState.reset();
    _vbo.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int gm::ShellApp::initialize() {
    using namespace gm;

    _window = SDL_CreateWindow("Grimm Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
    if (_window == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create window", nullptr);
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (!SDL_GetWindowWMInfo(_window.get(), &wmInfo)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not get window info", _window.get());
    }

#if GM_GPU_ENABLE_D3D11
    if (_device == nullptr) {
        auto factory = gpu::CreateFactoryD3D11();
        _device = factory->createDevice(0);
    }
#endif
    if (_device == nullptr) {
        auto factory = gpu::CreateFactoryNull();
        _device = factory->createDevice(0);
    }

    if (_device == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not find device", _window.get());
        return 1;
    }

#if GM_PLATFORM_WINDOWS
    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
#endif
    if (_swapChain == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Failed to create swap chain", _window.get());
        return 1;
    }

    _rtv = _device->createRenderTargetView(_swapChain->getBuffer(0).get());

    _commandList = _device->createCommandList();
    if (_commandList == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create command list", _window.get());
        return 1;
    }

    _vbo = _device->createBuffer(gpu::BufferType::Vertex, sizeof(triangle));
    _commandList->update(_vbo.get(), span{triangle, 6}.as_bytes(), 0);

    gpu::PipelineStateDesc pipelineDesc;

    auto stream = _fileSystem.openRead("build/resources/shaders/basic.vs_5_0.cbo");
    if (fs::readBlob(stream, pipelineDesc.vertShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open vertex shader", _window.get());
        return 1;
    }
    stream = _fileSystem.openRead("build/resources/shaders/basic.ps_5_0.cbo");
    if (fs::readBlob(stream, pipelineDesc.pixelShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open pixel shader", _window.get());
        return 1;
    }

    gpu::InputLayoutElement layout[2] = {
        {gpu::Format::R32G32B32Float, gpu::Semantic::Position, 0, 0},
        {gpu::Format::R32G32B32Float, gpu::Semantic::Color, 0, 0},
    };
    pipelineDesc.inputLayout = layout;
    _pipelineState = _device->createPipelineState(pipelineDesc);
    if (_pipelineState == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create pipeline state", _window.get());
        return 1;
    }

    blob imguiVertShader, imguiPixelShader;
    stream = _fileSystem.openRead("build/resources/shaders/imgui.vs_5_0.cbo");
    if (fs::readBlob(stream, imguiVertShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open imgui vertex shader", _window.get());
        return 1;
    }
    stream = _fileSystem.openRead("build/resources/shaders/imgui.ps_5_0.cbo");
    if (fs::readBlob(stream, imguiPixelShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open imgui pixel shader", _window.get());
        return 1;
    }

    ImGui::CreateContext();
    _drawImgui.createResources(*_device, ImGui::GetIO(), std::move(imguiVertShader), std::move(imguiPixelShader));

    return 0;
}

void gm::ShellApp::run() {
    while (isRunning()) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                switch (ev.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    onWindowClosed();
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    onWindowSizeChanged();
                    break;
                }
            }
        }

        gpu::Viewport viewport;
        int width, height;
        SDL_GetWindowSize(_window.get(), &width, &height);
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);

        auto& imguiIO = ImGui::GetIO();
        imguiIO.DisplaySize.x = viewport.width;
        imguiIO.DisplaySize.y = viewport.height;
        ImGui::NewFrame();
        ImGui::ShowDemoWindow();
        ImGui::Render();

        _commandList->clear();
        _commandList->clearRenderTarget(_rtv.get(), {0.f, 0.f, 0.1f, 1.f});
        _commandList->setPipelineState(_pipelineState.get());
        _commandList->bindRenderTarget(0, _rtv.get());
        _commandList->bindVertexBuffer(0, _vbo.get(), sizeof(PackedVector3f) * 2);
        _commandList->setPrimitiveTopology(gpu::PrimitiveTopology::Triangles);
        _commandList->setViewport(viewport);
        _commandList->draw(3);

        _drawImgui.draw(*ImGui::GetDrawData(), *_commandList);

        _commandList->finish();
        _device->execute(_commandList.get());

        _swapChain->present();
    }
}

void gm::ShellApp::quit() {
    _running = false;
}

void gm::ShellApp::onWindowClosed() {
    quit();
}

void gm::ShellApp::onWindowSizeChanged() {
    int width, height;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _rtv.reset();
    _commandList->clear();
    _swapChain->resizeBuffers(width, height);
    _rtv = _device->createRenderTargetView(_swapChain->getBuffer(0).get());
}
