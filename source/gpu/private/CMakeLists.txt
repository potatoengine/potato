target_sources(gpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/factory.cpp
)

add_subdirectory(null_backend)
add_subdirectory(vulkan_backend)
add_subdirectory(d3d12_backend)