#include "grimm/foundation/box.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/gpu/device.h"

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

private:
    gm::box<gm::IGPUDevice> _device;
    gm::unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
};
