include(FindPackageHandleStandardArgs)

if(NOT WIN32)
    check_cxx_source_compiles(
        "#include <filesystem>
        int main(int argc, char* argv[]) { std::filesystem::path p(\"test\"); return 0; }"
        HAS_FILESYSTEM
    )

    if(NOT "${HAS_FILESYSTEM}")
        # https://stackoverflow.com/questions/42593022/cmake-detect-which-library-libc-or-libstdc-is-configured-to-be-used-against
        check_cxx_source_compiles(
            "#include <iostream>
            int a =
            #ifdef __GLIBCXX__
                1;
            #else
                fgsfds;
            #endif
            int main(int argc, char* argv[]) { return 0; }"
            IS_LIBSTDCXX
        )

        if(IS_LIBSTDCXX)
            set(LIBFS "stdc++fs")
        else()
            set(LIBFS "c++fs")
        endif()

        FIND_PACKAGE_HANDLE_STANDARD_ARGS(Filesystem REQUIRED_VARS LIBFS)
    endif(NOT "${HAS_FILESYSTEM}")
endif()
