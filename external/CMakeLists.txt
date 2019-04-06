set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")
set(CMAKE_FOLDER "external")

add_subdirectory(litexx EXCLUDE_FROM_ALL)

set(FORMATXX_BUILD_TESTS OFF CACHE BOOL "enable formatxx tests")
add_subdirectory(formatxx EXCLUDE_FROM_ALL)

set(DOCTEST_WITH_TESTS OFF CACHE BOOL "enable doctest tests")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "enable doctest static lib")
add_subdirectory(doctest EXCLUDE_FROM_ALL)

add_subdirectory(spdlog EXCLUDE_FROM_ALL)

add_library(imgui)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
)
set_target_properties(imgui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(imgui PUBLIC
    IM_ASSERT=UP_ASSERT
    IMGUI_USER_CONFIG="potato/foundation/assertion.h"
)
target_link_libraries(imgui PUBLIC potato::foundation)

set(GLM_QUIET ON CACHE BOOL "quiet glm build")
set(GLM_TEST_ENABLE OFF CACHE BOOL "disable glm tests")
add_subdirectory(glm EXCLUDE_FROM_ALL)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_LIST_DIR}/stb")

set(JSON_MultipleHeaders ON CACHE INTERNAL "use multiple json header build")
set(JSON_BuildTests OFF CACHE INTERNAL "disable json tests")
add_subdirectory(json EXCLUDE_FROM_ALL)

if(WIN32 AND NOT TARGET SDL2)
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

    set(SDL2_PATH ${SDL2_SDK_PATH} CACHE INTERNAL "local SDL2 path")
    find_package(SDL2 REQUIRED)
endif()

set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})
