include(config.cmake)

FetchContent_Populate(libuv)
add_subdirectory(${libuv_SOURCE_DIR} ${libuv_BINARY_DIR} EXCLUDE_FROM_ALL)

# Silence warnings that libuv generates (ugh)
if(MSVC)
    target_compile_options(uv PRIVATE
        /wd4090 # const qualifiers
        /wd4244 # float to integer conversion
        /wd4245 # signed/unsigned mismatch
        /wd4267 # integer truncation
        /wd4701 # potentially unitialized value (!!!)
        /wd4702 # unreachable code
    )
endif()
