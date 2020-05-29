cmake_minimum_required(VERSION 3.16)
project(recon VERSION 0.1 LANGUAGES CXX)

include(up_copy_library_import)

if(TARGET potato_librecon)

    add_executable(potato_recon)
    add_executable(potato::recon ALIAS potato_recon)

    target_sources(potato_recon PRIVATE
        "private/main.cpp"
    )

    set_target_properties(potato_recon PROPERTIES OUTPUT_NAME recon)

    up_set_common_properties(potato_recon)

    target_link_libraries(potato_recon PRIVATE potato::librecon)

    target_link_libraries(potato_recon PRIVATE assimp)
    include(up_copy_library_import)
    up_copy_library_import(assimp potato_recon)
endif()
