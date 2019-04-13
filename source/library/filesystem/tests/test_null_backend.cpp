#include "potato/filesystem/null_backend.h"
#include "potato/filesystem/stream.h"
#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][filesystem] up::NullBackend") {
    using namespace up;
    using namespace up;

    DOCTEST_TEST_CASE("null abides") {
        auto null = NullBackend::create();

        DOCTEST_CHECK(!null.fileExists("/test.txt"));
        DOCTEST_CHECK(!null.directoryExists("/"));

        DOCTEST_CHECK(!null.openRead("/test.txt").isOpen());
        DOCTEST_CHECK(!null.openWrite("/test.txt").isOpen());

        auto cb(EnumerateCallback{[](FileInfo const&) { return EnumerateResult::Continue; }});
        DOCTEST_CHECK_EQ(null.enumerate("/", cb), EnumerateResult::Continue);

        DOCTEST_CHECK_EQ(null.createDirectories("/foo/bar"), IOResult::UnsupportedOperation);

        DOCTEST_CHECK_EQ(null.copyFile("/test.txt", "/out.txt"), IOResult::UnsupportedOperation);
    }
}
