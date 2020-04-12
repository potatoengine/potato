include(FetchContent)

FetchContent_Declare(doctest
    URL https://github.com/onqtam/doctest/archive/0b0a37405e878628009c9378e2fafa49e1499878.zip
    URL_HASH SHA1=3772e3bdb58f4280f28576911d0d36f7f240a4c7
)
FetchContent_Declare(imgui
    #URL https://github.com/ocornut/imgui/archive/v1.75.zip
    #URL_HASH SHA1=2f36c5f3d36e3640d3c85abd0bb7b7c89f59e9b3
    URL https://github.com/potatoengine/imgui/archive/up20200330.zip
    URL_HASH SHA1=be63bf493876e806230064180a9a04a3f6fc9f06
)
FetchContent_Declare(glm
    URL https://github.com/g-truc/glm/archive/d162eee1e6f7c317a09229fe6ceab8ec6ab9a4b4.zip
    URL_HASH SHA1=bb295dced578e1dfe246b33fd590b7be1231b8c3
)
FetchContent_Declare(stb
    URL https://github.com/nothings/stb/archive/052dce117ed989848a950308bd99eef55525dfb1.zip
    URL_HASH SHA1=1538ab6b5ed629f77f322e4ebbdef50f71233a90
)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/archive/53c3eefa2cf790a7130fed3e13a3be35c2f2ace2.zip
    URL_HASH SHA1=c97d1a8310d71da9913d09af437d4b193fe446e8
)
FetchContent_Declare(sdl2_vc_sdk
    URL https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip
    URL_HASH SHA1=0b4d2a9bd0c66847d669ae664c5b9e2ae5cc8f00
)
FetchContent_Declare(flatbuffers
    URL https://github.com/google/flatbuffers/archive/9e7e8cbe9f675123dd41b7c62868acad39188cae.zip
)
