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
        
    set(WWISE_RELEASE_LIBRARY_SUFFICES "${WWISE_ARCHITECTURE}_${WWISE_MSVC_VERSION}/Release/lib")
    set(WWISE_DEBUG_LIBRARY_SUFFICES "${WWISE_ARCHITECTURE}_${WWISE_MSVC_VERSION}/Debug/lib")

    find_path(WWISE_INCLUDE_DIR
        NAMES AK/AkWwiseSDKVersion.h
        PATH_SUFFIXES include
        HINTS ${WWISE_HINTS}
    )
    
    find_library(WWISE_LIBRARY_RELEASE
        NAMES "SFLib.lib"
        PATH_SUFFIXES ${WWISE_RELEASE_LIBRARY_SUFFICES}
        HINTS ${WWISE_HINTS}
    )

    find_library(WWISE_LIBRARY_DEBUG
        NAMES "SFLib.lib"
        PATH_SUFFIXES ${WWISE_DEBUG_LIBRARY_SUFFICES}
        HINTS ${WWISE_HINTS}
    )

    if(WWISE_INCLUDE_DIR AND WWISE_LIBRARY_RELEASE)
        add_library(Wwise IMPORTED STATIC GLOBAL)
        set_target_properties(Wwise PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${WWISE_INCLUDE_DIR}
        )

        if(WWISE_LIBRARY_DEBUG)
            set_target_properties(Wwise PROPERTIES
                IMPORTED_LOCATION_DEBUG ${WWISE_LIBRARY_DEBUG}
                IMPORTED_LOCATION_RELEASE ${WWISE_LIBRARY_RELEASE}
                MAP_IMPORTED_CONFIG_MINSIZEREL Release
                MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
            ) 
        else(WWISE_LIBRARY_DEBUG)
            set_target_properties(Wwise PROPERTIES
                IMPORTED_LOCATION "${WWISE_LIBRARY_RELEASE}"
            )
        endif(WWISE_LIBRARY_DEBUG)
    endif()
        
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(Wwise REQUIRED_VARS WWISE_INCLUDE_DIR WWISE_LIBRARY_RELEASE)
endfunction(FindWwise)

if(NOT TARGET Wwise)
    FindWwise()
endif(NOT TARGET Wwise)
