#include "shell_app.h"
#include "grimm/foundation/box.h"
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
    _device.reset();
    _window.reset();
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
    auto factory = CreateD3d12GPUFactory();
    _device = factory->createDevice(0);
#endif

    if (_device == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not find device", _window.get());
        return 1;
    }

    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
    if (_swapChain == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Failed to create swap chain", _window.get());
        return 1;
    }

    auto descriptorHeap = _device->createDescriptorHeap();
    if (descriptorHeap == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create descriptor heap", _window.get());
        return 1;
    }

    auto handle = descriptorHeap->getCpuHandle();
    auto offset = descriptorHeap->getCpuHandleSize();

    for (int n = 0; n < 2; n++) {
        auto buffer = _swapChain->getBuffer(0);

        _device->createRenderTargetView(buffer.get(), handle);
        handle += offset;
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
