#include "shell_app.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/vector.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/factory.h"
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
    if (_device == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not find device", _window.get());
        return 1;
    }

    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
    if (_swapChain == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Failed to create swap chain", _window.get());
        return 1;
    }
#endif

    return 0;
}

void gm::ShellApp::run() {
    SDL_Event ev;
    for (;;) {
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                switch (ev.window.type) {
                case SDL_WINDOWEVENT_CLOSE:
                    return;
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    int width, height;
                    SDL_GetWindowSize(_window.get(), &width, &height);
                    _swapChain->resizeBuffers(width, height);
                    break;
                }
                }
            }
        }

        _swapChain->present();
    }
}
