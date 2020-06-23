// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/null.h"
#include "potato/runtime/stream.h"

#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][runtime] up::NullFileSystem") {
    using namespace up;
    using namespace up;

    DOCTEST_TEST_CASE("null abides") {
        auto null = NullFileSystem();

        DOCTEST_CHECK(!null.fileExists("/test.txt"));
        DOCTEST_CHECK(!null.directoryExists("/"));

        DOCTEST_CHECK(!null.openRead("/test.txt").isOpen());
        DOCTEST_CHECK(!null.openWrite("/test.txt").isOpen());

        auto cb(EnumerateCallback{[](EnumerateItem const&) { return EnumerateResult::Continue; }});
        DOCTEST_CHECK_EQ(null.enumerate("/", cb), EnumerateResult::Continue);

        DOCTEST_CHECK_EQ(null.createDirectories("/foo/bar"), IOResult::UnsupportedOperation);

        DOCTEST_CHECK_EQ(null.copyFile("/test.txt", "/out.txt"), IOResult::UnsupportedOperation);
    }
}
