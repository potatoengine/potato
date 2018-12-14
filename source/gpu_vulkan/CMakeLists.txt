cmake_minimum_required(VERSION 3.11)
project(gpu_vulkan VERSION 0.1 LANGUAGES CXX)

add_library(gpu_vulkan)
add_library(grimm::gpu_vulkan ALIAS gpu_vulkan)

find_package(Vulkan)

if(Vulkan_FOUND)
message(STATUS "ENABLED Vulkan support")

set_target_properties(gpu_vulkan PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_FRAMEWORK_EXPORTS
)

target_compile_features(gpu_vulkan
    PUBLIC
        cxx_std_17
)

target_include_directories(gpu_vulkan
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/gpu_vulkan
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(gpu_vulkan
    PUBLIC
        grimm::foundation
        grimm::gpu
        Vulkan::Vulkan
    PRIVATE
        doctest
)

target_compile_definitions(gpu_vulkan
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(gpu_vulkan
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_sources(gpu_vulkan PRIVATE
    public/grimm/gpu_vulkan/_export.h
    public/grimm/gpu_vulkan/vk_device.h
)

target_sources(gpu_vulkan PRIVATE
    private/vk_device.cpp
)

add_subdirectory(tests)

else(Vulkan_FOUND)
message(STATUS "DISABLED Vulkan support")
endif(Vulkan_FOUND)