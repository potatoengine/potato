target_sources(gpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/com_ptr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/d3d12_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/direct3d.h
)
