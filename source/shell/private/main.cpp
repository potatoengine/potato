#include "grimm/foundation/vector.h"
#include "grimm/gpu/device.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_messagebox.h>

#if GM_USE_D3D12
#include "grimm/gpu_d3d12/d3d12_factory.h"
#endif

static void run_shell();

int main(int argc, char* argv[])
{
    SDL_Window* window = SDL_CreateWindow("Grimm Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);

#if GM_USE_D3D12
    SDL_SysWMinfo wmInfo;
    SDL_GetWindowWMInfo(window, &wmInfo);
    HWND hwnd = wmInfo.info.win.window;

    auto factory = gm::CreateD3d12Factory();
    auto device = factory->createDevice(0);
    if (device == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not find device", window);
        return 1;
    }
#endif

    run_shell();
    SDL_DestroyWindow(window);

    return 0;
}

static void run_shell()
{
    SDL_Event ev;
    while (SDL_WaitEvent(&ev))
    {
        switch (ev.type)
        {
        case SDL_QUIT:
            return;
        case SDL_WINDOWEVENT:
            switch (ev.window.type)
            {
            case SDL_WINDOWEVENT_CLOSE:
                return;
            }
        }
    }
}