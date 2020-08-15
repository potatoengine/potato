// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/resource_manifest.h"

#include <catch2/catch.hpp>

TEST_CASE("ResourceManifest", "[potato][runtime]") {
    using namespace up;

    SECTION("parse") {
        string_view input =
            "# Comment\n"
            "\n" // blank
            ".meta=value\n"
            ".key=var\n"
            ":ROOT_ID|LOGICAL_ID|LOGICAL_NAME|CONTENT_HASH|DEBUG_NAME\n"
            "0|0|0|DEAD|zero\n"
            "1|1|0|C0DE|one\n"
            ""_sv;
        ResourceManifest manifest;
        auto parseResult = ResourceManifest::parseManifest(input, manifest);
        CHECK(parseResult);

        CHECK(manifest.size() == 2);

        CHECK(manifest.records().front().rootId == 0);
        CHECK(manifest.records().front().hash == 0xDEAD);
        CHECK(string_view{manifest.records().front().filename} == "zero"_sv);

        CHECK(manifest.records().back().rootId == 1);
        CHECK(manifest.records().back().hash == 0xC0DE);
        CHECK(string_view{manifest.records().back().filename} == "one"_sv);
    }
}
