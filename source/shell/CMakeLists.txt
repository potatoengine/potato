cmake_minimum_required(VERSION 3.11)
project(shell VERSION 0.1 LANGUAGES CXX)

find_package(SDL2 REQUIRED)

add_executable(shell)

target_include_directories(shell
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(shell PRIVATE
    grimm::foundation
    grimm::gpu
    SDL2
    SDL2main
)

if(WIN32)
    get_target_property(SDL2_LIBRARY SDL2::SDL2 IMPORTED_LOCATION)
    get_filename_component(SDL2_LIBRARY_NAME "${SDL2_LIBRARY}" NAME)
    add_custom_command(TARGET shell POST_BUILD
        MAIN_DEPENDENCY "${SDL2_LIBRARY}"
        BYPRODUCTS "${SDL2_LIBRARY_NAME}"
        COMMENT "Copying SDL2 DLL"
        COMMAND "${CMAKE_COMMAND}" -E copy "${SDL2_LIBRARY}" "$<TARGET_FILE_DIR:shell>/${SDL2_LIBRARY_NAME}"
    )
endif()

add_subdirectory(public/grimm/shell)
add_subdirectory(private)
add_subdirectory(tests)
