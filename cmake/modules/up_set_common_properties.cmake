function(up_set_common_properties TARGET)
    get_target_property(TYPE ${TARGET} TYPE)
    string(TOUPPER ${TARGET} TARGET_UPPER)

    # Potato requires C++17
    target_compile_features(${TARGET} PUBLIC
        cxx_std_17
    )
    set_target_properties(${TARGET} PROPERTIES
        LINKER_LANGUAGE CXX
        CXX_STANDARD 17
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
    )

    # Trick MSVC into behaving like a standards-complient
    # compiler.
    target_compile_options(${TARGET} PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    )
    target_compile_options(${TARGET} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:inline>
    )

    # Disable C4141, since __forceinline apparently implies
    # inline, but Clang/GCC's [always_inline] does _not_
    # imply inline, so it needs to be present
    target_compile_options(${TARGET} PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/wd4141>
    )

    # Windows should always enable UNICODE support, though
    # we should never use TCHAR or the unadorned functions.
    # The goal is to ensure here we don't accidentally use
    # an unadorned function with our usual string types and
    # have things Just Compile(tm) incorrectly.
    target_compile_definitions(${TARGET} PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    )

    # Set visibility hidden on *nix targets to mimic
    # Windows', and also for smaller symbol tables during
    # the linking stage
    set_target_properties(${TARGET} PROPERTIES
        DEFINE_SYMBOL "UP_${TARGET_UPPER}_EXPORTS"
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    # Doctest settings to make things work well
    target_compile_definitions(${TARGET} PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
        DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
    )

    # Folder category
    if (${TARGET} MATCHES "^test_")
        set_target_properties(${TARGET} PROPERTIES FOLDER tests)
    elseif (${TYPE} STREQUAL "EXECUTABLE")
        set_target_properties(${TARGET} PROPERTIES FOLDER tools)
    else()
        set_target_properties(${TARGET} PROPERTIES FOLDER libraries)
    endif()

    # Library public include paths and private paths
    # for both library and executable targets
    target_include_directories(${TARGET}
        PUBLIC
            $<INSTALL_INTERFACE:public>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/private
    )

    # Set test output directory
    if (${TARGET} MATCHES "^test_")
        set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${UP_TEST_OUTPUT_DIRECTORY})
    endif()
endfunction()
