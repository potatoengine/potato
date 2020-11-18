include(config.cmake)

FetchContent_Populate(tracy)
add_library(tracy STATIC)
target_sources(tracy PRIVATE
    "${tracy_SOURCE_DIR}/TracyClient.cpp"
    "${tracy_SOURCE_DIR}/Tracy.hpp"
)
target_include_directories(tracy PUBLIC "${tracy_SOURCE_DIR}")
target_compile_definitions(tracy PUBLIC TRACY_ENABLE=1)
target_link_libraries(tracy PRIVATE ${CMAKE_DL_LIBS})
