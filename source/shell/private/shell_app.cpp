#include "shell_app.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/vector.h"
#include "grimm/gpu/descriptor_heap.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/factory.h"
#include "grimm/gpu/resource.h"
#include "grimm/gpu/swap_chain.h"

#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>

gm::ShellApp::~ShellApp() {
    _commandList.reset();
    _rtvHeap.reset();
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

#if GM_GPU_ENABLE_D3D12
    if (_device == nullptr) {
        auto d3d12Factory = CreateD3d12GPUFactory();
        _device = d3d12Factory->createDevice(0);
    }
#endif
#if GM_GPU_ENABLE_VULKAN
    if (_device == nullptr) {
        auto vulkanFactory = CreateVulkanGPUFactory();
        _device = vulkanFactory->createDevice(0);
    }
#endif

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

    _rtvHeap = _device->createDescriptorHeap();
    if (_rtvHeap == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create descriptor heap", _window.get());
        return 1;
    }

    auto [handle, offset] = _rtvHeap->getCpuHandle();

    for (int n = 0; n < 2; n++) {
        auto buffer = _swapChain->getBuffer(n);

        _device->createRenderTargetView(buffer.get(), handle);
        handle += offset;
    }

    _commandList = _device->createCommandList();
    if (_commandList == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create command list", _window.get());
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

        auto [handle, offset] = _rtvHeap->getCpuHandle();

        int frameIndex = _swapChain->getCurrentBufferIndex();
        _commandList->reset();
        _commandList->resourceBarrier(_swapChain->getBuffer(frameIndex).get(), GpuResourceState::Present, GpuResourceState::RenderTarget);
        _commandList->clearRenderTarget(handle + offset * frameIndex);
        _commandList->resourceBarrier(_swapChain->getBuffer(frameIndex).get(), GpuResourceState::RenderTarget, GpuResourceState::Present);
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
