set(GM_GPU_D3D11_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_command_list.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_command_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_resource_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_resource_view.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_swap_chain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_swap_chain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_platform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_platform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_pipeline_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_pipeline_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_texture.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d11_texture.cpp
)

if(GM_GPU_ENABLE_D3D11)
    message(STATUS "ENABLED Direct3D 11 support")
    target_sources(gpu PRIVATE ${GM_GPU_D3D11_SOURCES})
    target_compile_definitions(gpu PUBLIC GM_GPU_ENABLE_D3D11)
else()
    message(STATUS "DISABLED Direct3D 11 support")

    # HEADER_FILE_ONLY is not consistently working for me,
    # so I'm adding a fake target to get the .cpp files registered
    # but unbuilt

    #set_source_files_properties(${GM_GPU_D3D11_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    add_library(gpu_d3d11 INTERFACE)
    target_sources(gpu_d3d11 INTERFACE ${GM_GPU_D3D11_SOURCES})
endif()
