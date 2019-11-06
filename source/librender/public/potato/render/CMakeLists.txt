target_sources(potato_librender PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/camera.h
    ${CMAKE_CURRENT_SOURCE_DIR}/debug_draw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/draw_imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/image.h
    ${CMAKE_CURRENT_SOURCE_DIR}/loader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/material.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh.h
    ${CMAKE_CURRENT_SOURCE_DIR}/model.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pixel_format.h
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/shader.h
    ${CMAKE_CURRENT_SOURCE_DIR}/texture.h
)

target_sources(potato_librender PRIVATE
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
