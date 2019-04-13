INCLUDE(FindPackageHandleStandardArgs)

function(FindWwise)
    set(WWISE_MSVC_VERSION vc150)
    set(WWISE_ARCHITECTURE x64)

    set(WWISE_VERSIONS "2019.1.0.6947")

    set(WWISE_ROOT_DIR CACHE PATH "WWISE root directory")
    set(WWISE_HINTS "$ENV{WWISESDK}" "${WWISE_ROOT_DIR}")
    foreach(version ${WWISE_VERSIONS})
        list(APPEND WWISE_HINTS "$ENV{ProgramFiles}/Audiokinetic/Wwise 2019.1.0.6947/SDK")
        list(APPEND WWISE_HINTS "$ENV{ProgramW6432}/Audiokinetic/Wwise 2019.1.0.6947/SDK")
    endforeach()
        
    find_path(WWISE_INCLUDE_DIR
        NAMES AK/AkWwiseSDKVersion.h
        PATH_SUFFIXES include
        HINTS ${WWISE_HINTS}
    )

    find_path(WWISE_LIBRARY_DIR
        NAMES SFLib.lib
        PATH_SUFFIXES "${WWISE_ARCHITECTURE}_${WWISE_MSVC_VERSION}/Debug/lib"
        HINTS ${WWISE_HINTS}
    )

    if(WWISE_INCLUDE_DIR AND WWISE_LIBRARY_DIR)
        add_library(Wwise INTERFACE)
        target_include_directories(Wwise INTERFACE ${WWISE_INCLUDE_DIR})
        target_link_libraries(Wwise INTERFACE
            "${WWISE_LIBRARY_DIR}/SFLib.lib"
            "${WWISE_LIBRARY_DIR}/AkMemoryMgr.lib"
            "${WWISE_LIBRARY_DIR}/AkStreamMgr.lib"
            "${WWISE_LIBRARY_DIR}/AkMusicEngine.lib"
            "${WWISE_LIBRARY_DIR}/AkSpatialAudio.lib"
            "${WWISE_LIBRARY_DIR}/AkVorbisDecoder.lib"
            "${WWISE_LIBRARY_DIR}/AkSoundengine.lib"
            dinput8
            Winmm
            dsound
            dxguid
            xinput
        )
    endif()
        
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Wwise REQUIRED_VARS WWISE_INCLUDE_DIR WWISE_LIBRARY_DIR)
endfunction(FindWwise)

if(NOT TARGET Wwise)
    FindWwise()
endif(NOT TARGET Wwise)
