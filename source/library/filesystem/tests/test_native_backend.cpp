#include "potato/foundation/vector.h"
#include "potato/foundation/string.h"
#include "potato/filesystem/native_backend.h"
#include "potato/filesystem/stream.h"
#include <doctest/doctest.h>
#include <string>
#include <iostream>
#include <algorithm>

DOCTEST_TEST_SUITE("[potato][filesystem] up::NativeBackend") {
    using namespace up;
    using namespace up;

    DOCTEST_TEST_CASE("fileExists") {
        auto native = NativeBackend();

        DOCTEST_CHECK(native.fileExists("test.txt"));
        DOCTEST_CHECK(!native.fileExists("foobar.txt"));

        DOCTEST_CHECK(!native.fileExists("parent"));
    }

    DOCTEST_TEST_CASE("directoryExists") {
        auto native = NativeBackend();

        DOCTEST_CHECK(native.directoryExists("parent"));
        DOCTEST_CHECK(native.directoryExists("parent/child"));

        DOCTEST_CHECK(!native.directoryExists("parent/child/grandchild"));
        DOCTEST_CHECK(!native.directoryExists("test.txt"));
    }

    DOCTEST_TEST_CASE("openRead") {
        auto native = NativeBackend();

        auto inFile = native.openRead("test.txt", FileOpenMode::Text);
        DOCTEST_CHECK(inFile.isOpen());

        byte buffer[1024];
        span<byte> bspan(buffer);
        inFile.read(bspan);
        string_view text(bspan.as_chars().data(), bspan.size());

        DOCTEST_CHECK_EQ(text.first(15), "This is a test.");
    }

    DOCTEST_TEST_CASE("enumerate") {
        vector<string> const expected{
            "parent"_sv,
            "parent/child"_sv,
            "parent/child/hello.txt"_sv,
            "test.txt"_sv};

        auto native = NativeBackend();

        vector<string> entries;

        auto cb = [&entries](FileInfo const& info) {
            entries.push_back(info.path);
            return EnumerateResult::Recurse;
        };
        DOCTEST_CHECK_EQ(native.enumerate(".", cb), EnumerateResult::Continue);

        DOCTEST_REQUIRE_EQ(entries.size(), expected.size());

        // platforms aren't necessarily going to return entries in the same order
        std::sort(entries.begin(), entries.end());

        for (typename decltype(entries)::size_type i = 0, e = entries.size(); i != e; ++i) {
            DOCTEST_CHECK_EQ(entries[i], expected[i]);
        }
    }

    DOCTEST_TEST_CASE("stat") {
        auto native = NativeBackend();

        FileStat stat;
        auto rs = native.fileStat("test.txt", stat);
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
