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
#include "grimm/gpu/resource.h"
#include "grimm/gpu/swap_chain.h"
#include "grimm/math/packed.h"

#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>

static constexpr gm::PackedVector4f triangle[] = {
    {0, 0, 0, 1},
    {1, 1, 0, 1},
    {1, 0, 0, 1},
};

gm::ShellApp::~ShellApp() {
    _commandList.reset();
    _rtv.reset();
    _pipelineState.reset();
    _srv.reset();
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
        auto factory = CreateGPUFactoryD3D11();
        _device = factory->createDevice(0);
    }
#endif
    if (_device == nullptr) {
        auto factory = CreateNullGPUFactory();
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

    _vbo = _device->createBuffer(BufferType::Vertex, sizeof(triangle));
    _commandList->update(_vbo.get(), span{triangle, 3}.as_bytes(), 0);
    _srv = _device->createShaderResourceView(_vbo.get());

    GpuPipelineStateDesc pipelineDesc;

    auto stream = _fileSystem.openRead("build/resources/shaders/basic.vs_6_0.dxo");
    if (fs::readBlob(stream, pipelineDesc.vertShader) != fs::Result{}) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not open shader", _window.get());
        return 1;
    }

    _pipelineState = _device->createPipelineState(pipelineDesc);
    if (_pipelineState == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create pipeline state", _window.get());
        return 1;
    }

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
                switch (ev.window.type) {
                case SDL_WINDOWEVENT_CLOSE:
                    onWindowClosed();
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    onWindowSizeChanged();
                    break;
                }
            }
        }

        _commandList->clear();
        _commandList->clearRenderTarget(_rtv.get(), {1.f, 0.f, 0.f, 1.f});
        _commandList->setPipelineState(_pipelineState.get());
        _commandList->bindBuffer(0, _vbo.get(), sizeof(PackedVector4f));
        _commandList->setPrimitiveTopology(PrimitiveTopology::Triangles);
        _commandList->draw(3);
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
    _swapChain->resizeBuffers(width, height);
}
