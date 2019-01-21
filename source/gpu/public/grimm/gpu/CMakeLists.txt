target_sources(gpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/command_list.h
    ${CMAKE_CURRENT_SOURCE_DIR}/com_ptr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/descriptor_heap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/direct3d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/swap_chain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/transition.h
)
