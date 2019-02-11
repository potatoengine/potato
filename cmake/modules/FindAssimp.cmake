# https://raw.githubusercontent.com/assimp/assimp/270355f326ba80a2a3a0f64a2f3ece4932cbd987/cmake-modules/Findassimp.cmake
# modified by Sean Middleditch to use an import library, remove legacy unused features

INCLUDE(FindPackageHandleStandardArgs)

if(NOT TARGET assimp)
    if(WIN32)
        set(ASSIMP_ROOT_DIR CACHE PATH "ASSIMP root directory")

        # Find path of each library
        find_path(ASSIMP_INCLUDE_DIR
            NAMES
                assimp/anim.h
            HINTS
                "${ASSIMP_ROOT_DIR}/include"
                "$ENV{ProgramW6432}/Assimp/include"
        )

        set(ASSIMP_MSVC_VERSION vc140)
        set(ASSIMP_ARCHITECTURE x64)
    
        find_path(ASSIMP_LIBRARY_DIR
            NAMES
                assimp-${ASSIMP_MSVC_VERSION}-mt.lib
            HINTS
                "${ASSIMP_ROOT_DIR}/lib/${ASSIMP_ARCHITECTURE}"
                "$ENV{ProgramW6432}/Assimp/lib/${ASSIMP_ARCHITECTURE}"
        )
        
        find_library(ASSIMP_LIBRARY_RELEASE assimp-${ASSIMP_MSVC_VERSION}-mt.lib PATHS ${ASSIMP_LIBRARY_DIR})
        find_library(ASSIMP_LIBRARY_DEBUG assimp-${ASSIMP_MSVC_VERSION}-mtd.lib PATHS ${ASSIMP_LIBRARY_DIR})

        set(ASSIMP_DLL_PATH ${ASSIMP_LIBRARY_DIR}/../../bin/${ASSIMP_ARCHITECTURE})

        add_library(assimp IMPORTED SHARED GLOBAL)
        set_target_properties(assimp PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${ASSIMP_INCLUDE_DIR}
        )

        if(ASSIMP_LIBRARY_DEBUG)
            set_target_properties(assimp PROPERTIES
                IMPORTED_LOCATION_DEBUG ${ASSIMP_DLL_PATH}/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll
                IMPORTED_LOCATION_RELEASE ${ASSIMP_DLL_PATH}/assimp-${ASSIMP_MSVC_VERSION}-mt.dll
                IMPORTED_IMPLIB_DEBUG ${ASSIMP_LIBRARY_DEBUG}
                IMPORTED_IMPLIB_RELEASE ${ASSIMP_LIBRARY_RELEASE}
            )
        else(ASSIMP_LIBRARY_DEBUG)
            set_target_properties(assimp PROPERTIES
                IMPORTED_LOCATION ${ASSIMP_DLL_PATH}/assimp-${ASSIMP_MSVC_VERSION}-mt.dll
                IMPORTED_IMPLIB ${ASSIMP_LIBRARY_RELEASE}
            )
        endif(ASSIMP_LIBRARY_DEBUG)

        
        FIND_PACKAGE_HANDLE_STANDARD_ARGS(assimp REQUIRED_VARS ASSIMP_INCLUDE_DIR ASSIMP_LIBRARY_DIR)
    
    else(WIN32)

        find_path(
            assimp_INCLUDE_DIRS
            NAMES assimp/anim.h
            HINTS
                /usr/include/
                /usr/local/include/
        )

        find_library(
            assimp_LIBRARIES
            NAMES assimp
            HINTS
                /usr/lib/
                /usr/lib64/
                /usr/local/lib/
                /usr/local/lib64/
        )

        if(assimp_INCLUDE_DIRS AND assimp_LIBRARIES)
            set(assimp_FOUND TRUE)
        endif (assimp_INCLUDE_DIRS AND assimp_LIBRARIES)

        if(assimp_FOUND)
            if(NOT assimp_FIND_QUIETLY)
                message(STATUS "Found asset importer library: ${assimp_LIBRARIES}")
            endif (NOT assimp_FIND_QUIETLY)
        else(assimp_FOUND)
            if(assimp_FIND_REQUIRED)
                message(FATAL_ERROR "Could not find asset importer library")
            endif (assimp_FIND_REQUIRED)
        endif (assimp_FOUND)

        add_library(assimp IMPORTED SHARED GLOBAL)
        set_target_properties(assimp PROPERTIES
            IMPORTED_LOCATION ${assimp_LIBRARIES}
            INTERFACE_INCLUDE_DIRECTORIES ${assimp_INCLUDE_DIRS}
        )

        FIND_PACKAGE_HANDLE_STANDARD_ARGS(assimp REQUIRED_VARS assimp_INCLUDE_DIRS assimp_LIBRARIES)
    
    endif(WIN32)
endif(NOT TARGET assimp)
