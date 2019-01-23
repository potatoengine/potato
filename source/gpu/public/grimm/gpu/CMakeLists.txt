target_sources(gpu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/com_ptr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/command_list.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/device.h
    ${CMAKE_CURRENT_SOURCE_DIR}/factory.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pipeline_state.h
    ${CMAKE_CURRENT_SOURCE_DIR}/resource_view.h
    ${CMAKE_CURRENT_SOURCE_DIR}/sampler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/swap_chain.h
    ${CMAKE_CURRENT_SOURCE_DIR}/texture.h
)
