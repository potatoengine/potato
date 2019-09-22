include(FetchContent)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG 2.3.1
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_Declare(litexx
    GIT_REPOSITORY https://github.com/seanmiddleditch/litexx.git
    GIT_TAG 0.1.0
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_Declare(formatxx
    GIT_REPOSITORY https://github.com/seanmiddleditch/formatxx.git
    GIT_TAG 0.10.0
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.69
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.5
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG origin/master
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.6.1
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
)
FetchContent_Declare(sdl2_vc_sdk
    URL https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip
)
