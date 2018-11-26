#include "grimm/foundation/vector.h"
#include <SDL.h>

static void run_shell();

int main(int argc, char* argv[])
{
    gm::vector<int> v;

    SDL_Window* window = SDL_CreateWindow("Grimm Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
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