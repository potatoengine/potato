include(FetchContent)

FetchContent_Declare(doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG 2.4.0
)
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/potatoengine/imgui.git
    GIT_TAG up20200701
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
    GIT_TAG v3.8.0
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
