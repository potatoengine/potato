cmake_minimum_required(VERSION 3.11)
project(recon VERSION 0.1 LANGUAGES CXX)

include(gm_copy_library_import)

add_executable(recon)

target_include_directories(recon
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(recon PRIVATE
    grimm::foundation
    grimm::gpu
)

add_subdirectory(public/grimm/recon)
add_subdirectory(private)
add_subdirectory(tests)

find_package(Filesystem REQUIRED)
if(LIBFS)
    target_link_libraries(recon PUBLIC "${LIBFS}")
endif()

get_target_property(GM_GPU_ENABLE_D3D12 gpu GM_GPU_ENABLE_D3D12)
if(GM_GPU_ENABLE_D3D12)
    find_package(dxc REQUIRED)
    target_link_libraries(recon PRIVATE dxc::dxcompiler)
endif(GM_GPU_ENABLE_D3D12)

gm_copy_library_import(dxc::dxcompiler recon)
