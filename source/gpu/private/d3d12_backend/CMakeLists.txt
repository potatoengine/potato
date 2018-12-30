target_sources(gpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/com_ptr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_command_list.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_command_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_descriptor_heap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_descriptor_heap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_resource.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_swap_chain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_swap_chain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/direct3d.h
)
