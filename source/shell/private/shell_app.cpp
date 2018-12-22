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

    SDL_SysWMinfo wmInfo;
    SDL_GetWindowWMInfo(_window.get(), &wmInfo);

#if GM_GPU_ENABLE_D3D12
    auto factory = CreateD3d12GPUFactory();
    auto device = factory->createDevice(0);
    if (device == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not find device", _window.get());
        return 1;
    }
    auto swapChain = device->createSwapChain(wmInfo.info.win.window);
#endif

    return 0;
}

void gm::ShellApp::run() {
    SDL_Event ev;
    while (SDL_WaitEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            return;
        case SDL_WINDOWEVENT:
            switch (ev.window.type) {
            case SDL_WINDOWEVENT_CLOSE:
                return;
            }
        }
    }
}
