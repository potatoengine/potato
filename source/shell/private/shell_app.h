#include "grimm/foundation/box.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/swap_chain.h"

#include <SDL.h>

namespace gm {
    class ShellApp;
}

class gm::ShellApp {
public:
    ShellApp() = default;
    ~ShellApp();

    ShellApp(ShellApp const&) = delete;
    ShellApp& operator=(ShellApp const&) = delete;

    int initialize();
    void run();
    void quit();

    bool isRunning() const { return _running; }

private:
    void onWindowSizeChanged();
    void onWindowClosed();

private:
    bool _running = true;
    gm::box<gm::IGPUDevice> _device;
    gm::box<gm::ISwapChain> _swapChain;
    gm::unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
};
