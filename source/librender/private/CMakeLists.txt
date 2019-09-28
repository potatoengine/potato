target_sources(potato_librender PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/debug_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/draw_imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/material.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/texture.cpp
)

add_subdirectory(null_backend)
add_subdirectory(d3d11_backend)
