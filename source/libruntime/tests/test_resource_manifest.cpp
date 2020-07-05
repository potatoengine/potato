// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/resource_manifest.h"

#include <doctest/doctest.h>

DOCTEST_TEST_SUITE("[potato][runtime] ResourceManifest") {
    using namespace up;

    DOCTEST_TEST_CASE("parse") {
        string_view input =
            "# Comment\n"
            "\n" // blank
            ".meta=value\n"
            ".key=var\n"
            ":ID|HASH|NAME\n"
            "0|DEAD|zero\n"
            "1|C0DE|one\n"
            ""_sv;
        ResourceManifest manifest;
        auto parseResult = ResourceManifest::parseManifest(input, manifest);
        DOCTEST_CHECK(parseResult);

        DOCTEST_CHECK_EQ(2, manifest.size());

        DOCTEST_CHECK_EQ(0, manifest.records().front().id);
        DOCTEST_CHECK_EQ(0xDEAD, manifest.records().front().hash);
        DOCTEST_CHECK_EQ("zero"_sv, string_view{manifest.records().front().filename});

        DOCTEST_CHECK_EQ(1, manifest.records().back().id);
        DOCTEST_CHECK_EQ(0xC0DE, manifest.records().back().hash);
        DOCTEST_CHECK_EQ("one"_sv, string_view{manifest.records().back().filename});
    }
}
