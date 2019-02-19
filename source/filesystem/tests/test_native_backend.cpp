#include "grimm/foundation/vector.h"
#include "grimm/foundation/string_blob.h"
#include "grimm/filesystem/native_backend.h"
#include "grimm/filesystem/stream.h"
#include "doctest.h"
#include <string>
#include <iostream>
#include <algorithm>

DOCTEST_TEST_SUITE("[grimm][filesystem] gm::fs::NativeBackend") {
    using namespace gm;
    using namespace gm::fs;

    DOCTEST_TEST_CASE("fileExists") {
        auto native = NativeBackend::create();

        DOCTEST_CHECK(native.fileExists("test.txt"));
        DOCTEST_CHECK(!native.fileExists("foobar.txt"));

        DOCTEST_CHECK(!native.fileExists("parent"));
    }

    DOCTEST_TEST_CASE("directoryExists") {
        auto native = NativeBackend::create();

        DOCTEST_CHECK(native.directoryExists("parent"));
        DOCTEST_CHECK(native.directoryExists("parent/child"));

        DOCTEST_CHECK(!native.directoryExists("parent/child/grandchild"));
        DOCTEST_CHECK(!native.directoryExists("test.txt"));
    }

    DOCTEST_TEST_CASE("openRead") {
        auto native = NativeBackend::create();

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

        auto native = NativeBackend::create();

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
        auto native = NativeBackend::create();

        FileStat stat;
        auto rs = native.fileStat("test.txt", stat);
        DOCTEST_CHECK_EQ(rs, Result::Success);
        DOCTEST_CHECK_EQ(stat.type, FileType::Regular);

        // note: can't test size (Windows/UNIX line endings!) or mtime (git)
    }

    // TODO:
    // Figure out how to reliably test "write" operations on the native backend.
    // - [ ] test openWrite
    // - [ ] test createDirectories
    // - [ ] test copyFile
}
