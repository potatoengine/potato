#include "grimm/filesystem/null_filesystem.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][filesystem] gm::fs::NullFilesystemBackend") {
    using namespace gm;
    using namespace gm::fs;

    DOCTEST_TEST_CASE("null abides") {
        NullFileSystemBackend null;

        DOCTEST_CHECK(!null.fileExists("/test.txt"));
        DOCTEST_CHECK(!null.directoryExists("/"));
    }
}
