set(GM_GPU_VULKAN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_swap_chain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_swap_chain.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vkn_pipeline_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vulkan.h
)

target_compile_definitions(gpu PRIVATE VULKAN_HPP_NO_EXCEPTIONS)

if(GM_GPU_ENABLE_VULKAN)
    message(STATUS "ENABLED Vulkan support")
    target_sources(gpu PRIVATE ${GM_GPU_VULKAN_SOURCES})
    target_compile_definitions(gpu PUBLIC GM_GPU_ENABLE_VULKAN)
else()
    message(STATUS "DISABLED Vulkan support")

    # HEADER_FILE_ONLY is not consistently working for me,
    # so I'm adding a fake target to get the .cpp files registered
    # but unbuilt

    #set_source_files_properties(${GM_GPU_VULKAN_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
    add_library(gpu_vulkan_disabled INTERFACE)
    target_sources(gpu_vulkan_disabled INTERFACE ${GM_GPU_VULKAN_SOURCES})
    target_link_libraries(gpu_vulkan_disabled INTERFACE grimm::foundation grimm::gpu)
endif()
