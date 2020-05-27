include(config.cmake)

FetchContent_Populate(soloud)

add_library(soloud STATIC)
target_include_directories(soloud PUBLIC "${soloud_SOURCE_DIR}/include")
target_compile_definitions(soloud
    PRIVATE
        "WITH_SDL2_STATIC=$<PLATFORM_ID:Windows>"
        WITH_NULL=1
        _CRT_SECURE_NO_WARNINGS=1
)
target_sources(soloud PRIVATE
    ${soloud_SOURCE_DIR}/src/core/soloud.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_audiosource.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_bus.cpp
    ${soloud_SOURCE_DIR}/src/core/soloud_core_3d.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_core_basicops.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_core_faderops.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_core_filterops.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_core_getters.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_core_setters.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_core_voicegroup.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_core_voiceops.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_fader.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_fft.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_fft_lut.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_file.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_filter.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_misc.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_queue.cpp
	${soloud_SOURCE_DIR}/src/core/soloud_thread.cpp
)
target_sources(soloud PRIVATE
    ${soloud_SOURCE_DIR}/src/backend/null/soloud_null.cpp
)
target_sources(soloud PRIVATE
    ${soloud_SOURCE_DIR}/src/backend/sdl/soloud_sdl2.cpp
)
target_sources(soloud PRIVATE
    ${soloud_SOURCE_DIR}/src/audiosource/wav/dr_impl.cpp
    ${soloud_SOURCE_DIR}/src/audiosource/wav/soloud_wav.cpp
    ${soloud_SOURCE_DIR}/src/audiosource/wav/soloud_wavstream.cpp
    ${soloud_SOURCE_DIR}/src/audiosource/wav/stb_vorbis.c
)

if(WIN32)
    find_package(SDL2 REQUIRED)
    target_link_libraries(soloud PRIVATE SDL2)
    target_sources(soloud PRIVATE
        ${soloud_SOURCE_DIR}/src/backend/sdl2_static/soloud_sdl2_static.cpp
    )
endif()
