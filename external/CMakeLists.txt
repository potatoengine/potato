# Declared dependencies
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

# Build dependencies
message(STATUS "Updating dependencies (this might take some time!)")

set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")
set(CMAKE_FOLDER "external")
set(UP_DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps")
set(FETCHCONTENT_QUIET OFF CACHE BOOL "Disable fetch content noise")

FetchContent_Populate(doctest)
set(DOCTEST_WITH_TESTS OFF CACHE BOOL "enable doctest tests")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "enable doctest static lib")
add_subdirectory(${doctest_SOURCE_DIR} ${doctest_BINARY_DIR} EXCLUDE_FROM_ALL)

FetchContent_Populate(litexx)
add_subdirectory(${litexx_SOURCE_DIR} ${litexx_BINARY_DIR} EXCLUDE_FROM_ALL)

FetchContent_Populate(formatxx)
add_subdirectory(${formatxx_SOURCE_DIR} ${formatxx_BINARY_DIR} EXCLUDE_FROM_ALL)
set(FORMATXX_BUILD_TESTS OFF CACHE BOOL "enable formatxx tests")

FetchContent_Populate(imgui)
add_library(imgui)
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
target_sources(imgui PRIVATE
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
)
set_target_properties(imgui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(imgui PUBLIC
    IM_ASSERT=UP_ASSERT
    IMGUI_USER_CONFIG="potato/core/assertion.h"
)
target_link_libraries(imgui PUBLIC potato::core)

FetchContent_Populate(glm)
set(GLM_QUIET ON CACHE BOOL "quiet glm build")
set(GLM_TEST_ENABLE OFF CACHE BOOL "disable glm tests")
add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR} EXCLUDE_FROM_ALL)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)

FetchContent_Populate(stb)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")

FetchContent_Populate(json)
set(JSON_MultipleHeaders ON CACHE INTERNAL "use multiple json header build")
set(JSON_BuildTests OFF CACHE INTERNAL "disable json tests")
add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)

if(WIN32)
    find_package(SDL2 QUIET)

    if (NOT TARGET SDL2)
        FetchContent_Populate(sdl2_vc_sdk)

        set(SDL2_PATH ${sdl2_vc_sdk_SOURCE_PATH} CACHE PATH "local SDL2 path" FORCE)
        find_package(SDL2 REQUIRED)
    endif()
endif()

set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})

message(STATUS "Finished updating dependencies")
