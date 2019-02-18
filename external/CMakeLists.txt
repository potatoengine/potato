set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")
set(CMAKE_FOLDER "external")

set(FMT_TEST OFF CACHE BOOL "enable fmtlib tests")
add_subdirectory(fmt-5.3.0 EXCLUDE_FROM_ALL)
set_target_properties(fmt PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

set(DOCTEST_WITH_TESTS OFF CACHE BOOL "enable doctest tests")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "enable doctest static lib")
add_subdirectory(doctest-2.2.0 EXCLUDE_FROM_ALL)

add_subdirectory(spdlog-1.3.0 EXCLUDE_FROM_ALL)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE rapidjson-1.1.0/include)
target_compile_definitions(rapidjson INTERFACE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

add_library(imgui)
target_include_directories(imgui PUBLIC imgui-1.67)
target_sources(imgui PRIVATE
    imgui-1.67/imgui.cpp
    imgui-1.67/imgui_demo.cpp
    imgui-1.67/imgui_draw.cpp
    imgui-1.67/imgui_widgets.cpp
)
set_target_properties(imgui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(imgui PUBLIC
    IM_ASSERT=GM_ASSERT
    IMGUI_USER_CONFIG="grimm/foundation/assertion.h"
)
target_link_libraries(imgui PUBLIC grimm::foundation)

set(GLM_QUIET ON CACHE BOOL "quiet glm build")
set(GLM_TEST_ENABLE OFF CACHE BOOL "disable glm tests")
add_subdirectory(glm-0.9.9.3/glm EXCLUDE_FROM_ALL)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE "${CMAKE_CURRENT_LIST_DIR}/stb-e6afb9c")

set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})
