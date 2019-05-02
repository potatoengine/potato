set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")
set(CMAKE_FOLDER "external")

include(FetchContent)

FetchContent_Populate(doctest
    GIT_REPOSITORY https://github.com/onqtam/doctest.git
    GIT_TAG 2.3.1
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_PROGRESS ON
    FETCHCONTENT_UPDATES_DISCONNECTED ON
    QUIET ON
    EXCLUDE_FROM_ALL OFF
)
set(DOCTEST_WITH_TESTS OFF CACHE BOOL "enable doctest tests")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "enable doctest static lib")
add_subdirectory(${doctest_SOURCE_DIR} ${doctest_BINARY_DIR} EXCLUDE_FROM_ALL)

FetchContent_Populate(litexx
    GIT_REPOSITORY https://github.com/seanmiddleditch/litexx.git
    GIT_TAG 19c31a3d1cdd7ca5ba99c28b7d634cc9e2e97539
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_PROGRESS ON
    FETCHCONTENT_UPDATES_DISCONNECTED ON
    QUIET ON
    EXCLUDE_FROM_ALL OFF
)
add_subdirectory(${litexx_SOURCE_DIR} ${litexx_BINARY_DIR} EXCLUDE_FROM_ALL)

FetchContent_Populate(formatxx
    GIT_REPOSITORY https://github.com/seanmiddleditch/formatxx.git
    GIT_TAG 15b7b71e3c1ce23224a7fc1f94121f99f321e0a4
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_PROGRESS ON
    FETCHCONTENT_UPDATES_DISCONNECTED ON
    QUIET ON
    EXCLUDE_FROM_ALL OFF
)
add_subdirectory(${formatxx_SOURCE_DIR} ${formatxx_BINARY_DIR} EXCLUDE_FROM_ALL)
set(FORMATXX_BUILD_TESTS OFF CACHE BOOL "enable formatxx tests")

FetchContent_Populate(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.69
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_PROGRESS ON
    FETCHCONTENT_UPDATES_DISCONNECTED ON
    QUIET ON
    EXCLUDE_FROM_ALL OFF
)
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
    IMGUI_USER_CONFIG="potato/foundation/assertion.h"
)
target_link_libraries(imgui PUBLIC potato::foundation)

FetchContent_Populate(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.5
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_PROGRESS ON
    FETCHCONTENT_UPDATES_DISCONNECTED ON
    QUIET ON
    EXCLUDE_FROM_ALL OFF
)
set(GLM_QUIET ON CACHE BOOL "quiet glm build")
set(GLM_TEST_ENABLE OFF CACHE BOOL "disable glm tests")
add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR} EXCLUDE_FROM_ALL)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)

FetchContent_Populate(stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG origin/master
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_PROGRESS ON
    FETCHCONTENT_UPDATES_DISCONNECTED ON
    QUIET ON
    EXCLUDE_FROM_ALL OFF
)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}")

FetchContent_Populate(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.6.1
    GIT_SHALLOW ON
    GIT_SUBMODULES ""
    GIT_PROGRESS ON
    FETCHCONTENT_UPDATES_DISCONNECTED ON
    QUIET ON
    EXCLUDE_FROM_ALL OFF
)
set(JSON_MultipleHeaders ON CACHE INTERNAL "use multiple json header build")
set(JSON_BuildTests OFF CACHE INTERNAL "disable json tests")
add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)

if(WIN32)
    find_package(SDL2 QUIET)

    if (NOT TARGET SDL2)
        set(SDL2_SDK_URL https://www.libsdl.org/release/SDL2-devel-2.0.9-VC.zip)
        set(SDL2_SDK_ZIP ${CMAKE_CURRENT_BINARY_DIR}/SDL2-2.0.9.zip)
        set(SDL2_SDK_PATH ${CMAKE_CURRENT_BINARY_DIR}/SDL2-2.0.9)

        if(NOT IS_DIRECTORY ${SDL2_SDK_PATH})
            message(STATUS "Downloading `${SDL2_SDK_URL}'")
            file(DOWNLOAD ${SDL2_SDK_URL} ${SDL2_SDK_ZIP})

            message(STATUS "Unpacking `${SDL2_SDK_ZIP}'")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E tar xzf ${SDL2_SDK_ZIP}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            )

            if(NOT IS_DIRECTORY ${SDL2_SDK_PATH})
                message(FATAL_ERROR "Failed to unpack `${SDL2_SDK_ZIP}' to `${SDL2_SDK_PATH}'")
            endif()
        endif()

        set(SDL2_PATH ${SDL2_SDK_PATH} CACHE INTERNAL "local SDL2 path" FORCE)
        find_package(SDL2 REQUIRED)
    endif()
endif()

set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})
