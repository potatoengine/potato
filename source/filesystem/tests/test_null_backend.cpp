#include "grimm/filesystem/null_backend.h"
#include "doctest.h"

DOCTEST_TEST_SUITE("[grimm][filesystem] gm::fs::NullBackend") {
    using namespace gm;
    using namespace gm::fs;

    DOCTEST_TEST_CASE("null abides") {
        auto null = NullBackend::create();

        DOCTEST_CHECK(!null.fileExists("/test.txt"));
        DOCTEST_CHECK(!null.directoryExists("/"));
    }
}
