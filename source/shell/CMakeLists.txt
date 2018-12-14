cmake_minimum_required(VERSION 3.11)
project(shell VERSION 0.1 LANGUAGES CXX)

add_executable(shell)

target_include_directories(shell
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(shell PRIVATE
    grimm::foundation
    SDL2
    SDL2main
)

if(TARGET grimm::gpu_vulkan)
    target_compile_definitions(shell PRIVATE GM_USE_VULKAN)
    target_link_libraries(shell PRIVATE grimm::gpu_vulkan)
endif()

if(TARGET grimm::gpu_d3d12)
    target_compile_definitions(shell PRIVATE GM_USE_D3D12)
    target_link_libraries(shell PRIVATE grimm::gpu_d3d12)
endif()

add_subdirectory(public/grimm/shell)
add_subdirectory(private)
add_subdirectory(tests)