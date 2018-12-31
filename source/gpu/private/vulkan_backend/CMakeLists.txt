target_sources(gpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_swap_chain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_swap_chain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan.h
)

target_compile_definitions(gpu PRIVATE
    VULKAN_HPP_NO_EXCEPTIONS
)
