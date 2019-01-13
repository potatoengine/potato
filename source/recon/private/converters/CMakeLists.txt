target_sources(recon_lib PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/convert_copy.h"
    "${CMAKE_CURRENT_LIST_DIR}/convert_copy.cpp"
)

# FIXME just mark these as header-only once I can make that work
if(GM_GPU_ENABLE_D3D12)
    target_sources(recon_lib PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/convert_hlsl.h"
        "${CMAKE_CURRENT_LIST_DIR}/convert_hlsl.cpp"
    )
endif()
