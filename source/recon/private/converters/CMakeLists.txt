target_sources(recon PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/convert_copy.h"
    "${CMAKE_CURRENT_LIST_DIR}/convert_copy.cpp"
    "$<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/convert_hlsl.h>"
    "$<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/convert_hlsl.cpp>"
)
