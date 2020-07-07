// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/filesystem.h"
#include "potato/runtime/stream.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

#include <doctest/doctest.h>
#include <algorithm>
#include <iostream>
#include <string>

DOCTEST_TEST_SUITE("[potato][runtime] up::FileSystem") {
    using namespace up;

    DOCTEST_TEST_CASE("fileExists") {
        DOCTEST_CHECK(FileSystem::fileExists("test.txt"));
        DOCTEST_CHECK(!FileSystem::fileExists("foobar.txt"));

        DOCTEST_CHECK(!FileSystem::fileExists("parent"));
    }

    DOCTEST_TEST_CASE("directoryExists") {
        DOCTEST_CHECK(FileSystem::directoryExists("parent"));
        DOCTEST_CHECK(FileSystem::directoryExists("parent/child"));

        DOCTEST_CHECK(!FileSystem::directoryExists("parent/child/grandchild"));
        DOCTEST_CHECK(!FileSystem::directoryExists("test.txt"));
    }

    DOCTEST_TEST_CASE("openRead") {
        auto inFile = FileSystem::openRead("test.txt", FileOpenMode::Text);
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

        auto cb = [&entries](EnumerateItem const& item) {
            entries.push_back(item.info.path);
            return EnumerateResult::Recurse;
        };
        DOCTEST_CHECK_EQ(FileSystem::enumerate(".", cb), EnumerateResult::Continue);

        DOCTEST_REQUIRE_EQ(entries.size(), expected.size());

        // platforms aren't necessarily going to return entries in the same order
        std::sort(entries.begin(), entries.end());

        for (typename decltype(entries)::size_type i = 0, e = entries.size(); i != e; ++i) {
            DOCTEST_CHECK_EQ(entries[i], expected[i]);
        }
    }

    DOCTEST_TEST_CASE("stat") {
        FileStat stat;
        auto rs = FileSystem::fileStat("test.txt", stat);
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
