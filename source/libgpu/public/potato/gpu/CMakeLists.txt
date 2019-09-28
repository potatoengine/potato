target_sources(potato_libgpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_command_list.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_pipeline_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_resource_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_sampler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_swap_chain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gpu_texture.h
)
