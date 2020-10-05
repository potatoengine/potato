include(FetchContent)

FetchContent_Declare(catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v2.13.0
)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/potatoengine/imgui.git
    GIT_TAG up20201004
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
    URL https://www.libsdl.org/release/SDL2-devel-2.0.12-VC.zip
    URL_HASH SHA1=6839B6EC345EF754A6585AB24F04E125E88C3392
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
    GIT_TAG v0.9-beta
)
