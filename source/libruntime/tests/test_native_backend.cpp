// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/filesystem.h"
#include "potato/runtime/stream.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <doctest/doctest.h>
#include <algorithm>
#include <iostream>
#include <string>

DOCTEST_TEST_SUITE("[potato][runtime] up::fs") {
    using namespace up;
    using namespace up::fs;

    DOCTEST_TEST_CASE("fileExists") {
        DOCTEST_CHECK(fileExists("test.txt"));
        DOCTEST_CHECK(!fileExists("foobar.txt"));

        DOCTEST_CHECK(!fileExists("parent"));
    }

    DOCTEST_TEST_CASE("directoryExists") {
        DOCTEST_CHECK(directoryExists("parent"));
        DOCTEST_CHECK(directoryExists("parent/child"));

        DOCTEST_CHECK(!directoryExists("parent/child/grandchild"));
        DOCTEST_CHECK(!directoryExists("test.txt"));
    }

    DOCTEST_TEST_CASE("openRead") {
        auto inFile = openRead("test.txt", OpenMode::Text);
        DOCTEST_CHECK(inFile.isOpen());

        byte buffer[1024];
        span<byte> bspan(buffer);
        inFile.read(bspan);
        string_view text(bspan.as_chars().data(), bspan.size());

        DOCTEST_CHECK_EQ(text.first(15), "This is a test.");
    }

    DOCTEST_TEST_CASE("enumerate") {
        vector<string> const expected{"parent"_sv, "parent/child"_sv, "parent/child/hello.txt"_sv, "test.txt"_sv};

        vector<string> entries;

        auto cb = [&entries](auto const& item, int) {
            entries.push_back(item.path);
            return recurse;
        };
        DOCTEST_CHECK_EQ(enumerate(".", cb), next);

        DOCTEST_REQUIRE_EQ(entries.size(), expected.size());

        // platforms aren't necessarily going to return entries in the same order
        std::sort(entries.begin(), entries.end());

        for (typename decltype(entries)::size_type i = 0, e = entries.size(); i != e; ++i) {
            DOCTEST_CHECK_EQ(entries[i], expected[i]);
        }
    }

    DOCTEST_TEST_CASE("stat") {
        auto const [rs, stat] = fileStat("test.txt");
        DOCTEST_CHECK_EQ(rs, IOResult::Success);
        DOCTEST_CHECK_EQ(stat.type, FileType::Regular);

        // note: can't test size (Windows/UNIX line endings!) or mtime (git)
    }

    // TODO:
    // Figure out how to reliably test "write" operations on the native backend.
    // - [ ] test openWrite
    // - [ ] test createDirectories
    // - [ ] test copyFile
}
