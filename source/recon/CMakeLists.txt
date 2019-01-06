cmake_minimum_required(VERSION 3.11)
project(recon VERSION 0.1 LANGUAGES CXX)

add_executable(recon)

target_include_directories(recon
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(recon PRIVATE
    grimm::foundation
    grimm::gpu
)

add_subdirectory(public/grimm/recon)
add_subdirectory(private)
add_subdirectory(tests)
