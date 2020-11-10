function(FindDXC)
    set(DXC_SEARCH_PATHS
	    ${DXC_PATH}
    )

    find_path(DXC_INCLUDE_DIR dxc/dxcapi.h
	    HINTS $ENV{DXCDIR}
	    PATH_SUFFIXES include
	    PATHS ${DXC_SEARCH_PATHS}
    )

    find_library(DXIL_IMPLIB_PATH
	    NAMES dxil.lib
	    HINTS $ENV{DXCDIR}
	    PATH_SUFFIXES lib
	    PATHS ${DXC_SEARCH_PATHS}
    )
    find_library(DXIL_DLL_PATH
	    NAMES dxil
	    HINTS $ENV{DXCDIR}
	    PATH_SUFFIXES bin
	    PATHS ${DXC_SEARCH_PATHS}
    )

    find_library(DXCOMPILER_IMPLIB_PATH
	    NAMES dxcompiler.lib
	    HINTS $ENV{DXCDIR}
	    PATH_SUFFIXES lib
	    PATHS ${DXC_SEARCH_PATHS}
    )
    find_library(DXCOMPILER_DLL_PATH
	    NAMES dxcompiler.dll
	    HINTS $ENV{DXCDIR}
	    PATH_SUFFIXES bin
	    PATHS ${DXC_SEARCH_PATHS}
    )

    find_program(DXC_BIN_PATH
        NAMES dxc.exe
	    HINTS $ENV{DXCDIR}
	    PATH_SUFFIXES bin
	    PATHS ${DXC_SEARCH_PATHS}
    )

    if(NOT DXCOMPILER_DLL_PATH)
        string(REPLACE ".lib" ".dll" DXCOMPILER_DLL_PATH ${DXCOMPILER_IMPLIB_PATH})
        string(REPLACE "/lib/" "/bin/" DXCOMPILER_DLL_PATH ${DXCOMPILER_DLL_PATH})
    endif()

    add_library(dxcompiler IMPORTED SHARED GLOBAL)
    add_library(dxc::dxcompiler ALIAS dxcompiler)
    set_target_properties(dxcompiler PROPERTIES IMPORTED_LOCATION ${DXCOMPILER_DLL_PATH} IMPORTED_IMPLIB ${DXCOMPILER_IMPLIB_PATH})
    set_target_properties(dxcompiler PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${DXC_INCLUDE_DIR})

    add_executable(dxc IMPORTED GLOBAL)
    set_target_properties(dxc PROPERTIES IMPORTED_LOCATION ${DXC_BIN_PATH})
    add_executable(dxc::dxc ALIAS dxc)

    if(DXIL_PATH)
        add_library(dxil IMPORTED SHARED GLOBAL)
        add_library(dxc::dxil ALIAS dxil)
        set_target_properties(dxil PROPERTIES IMPORTED_LOCATION ${DXIL_DLL_PATH})
        set_target_properties(dxil PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${DXC_INCLUDE_DIR} IMPORTED_IMPLIB ${DXIL_IMPLIB_PATH})

        set_target_properties(dxcompiler PROPERTIES IMPORTED_LINK_INTERFACE_LIBRARIES dxil)
    endif()

    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(DXC REQUIRED_VARS DXC_INCLUDE_DIR DXCOMPILER_DLL_PATH DXCOMPILER_IMPLIB_PATH)
endfunction()

if(NOT TARGET dxcompiler)
    FindDXC()
endif()
