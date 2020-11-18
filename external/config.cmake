include(FetchContent)

FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.0
)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/potatoengine/imgui.git
    GIT_TAG up20201103
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/potatoengine/stb.git
    GIT_TAG up20200529
)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.9.1
)
FetchContent_Declare(sdl2_vc_sdk
    URL https://github.com/potatoengine/win-sdks/releases/download/sdl2-2.0.12-x64/SDL2-2.0.12-win-x64.7z
    URL_HASH SHA1=21EFE9F45962EF2B57DC97FE7905D1EC82670AEF
)
FetchContent_Declare(assimp_win64_sdk
    URL https://github.com/potatoengine/win-sdks/releases/download/assimp-5.0.1-x64/assimp-5.0.1-win-x64.7z
    URL_HASH SHA1=8D96964E9E6946D39E187D6740BC5D2B21408BE2
)
FetchContent_Declare(flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v1.12.0
)
FetchContent_Declare(soloud
    GIT_REPOSITORY https://github.com/jarikomppa/soloud.git
    GIT_TAG RELEASE_20200207
)
FetchContent_Declare(nfd 
    GIT_REPOSITORY https://github.com/mlabbe/nativefiledialog.git
    GIT_TAG release_116
)
FetchContent_Declare(sapc
    GIT_REPOSITORY https://github.com/potatoengine/sapc.git
    GIT_TAG v0.10-beta
)
FetchContent_Declare(reproc
    GIT_REPOSITORY https://github.com/DaanDeMeyer/reproc.git
    GIT_TAG v14.2.1
)
FetchContent_Declare(tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git
    GIT_TAG v0.7.4
)
