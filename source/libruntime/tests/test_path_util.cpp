// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/path.h"

#include <doctest/doctest.h>
#include <iostream>

DOCTEST_TEST_SUITE("[potato][runtime] up::path") {
    using namespace up;
    using namespace up::path;

    DOCTEST_TEST_CASE("extension") {
        DOCTEST_CHECK_EQ(extension(zstring_view("")), "");
        DOCTEST_CHECK_EQ(extension(zstring_view("/foo/bar.txt")), ".txt");
        DOCTEST_CHECK_EQ(extension(zstring_view(".txt")), ".txt");
        DOCTEST_CHECK_EQ(extension(zstring_view("/foo/bar")), "");

        DOCTEST_CHECK_EQ(extension(string_view("")), "");
        DOCTEST_CHECK_EQ(extension(string_view("/foo/bar.txt")), ".txt");
        DOCTEST_CHECK_EQ(extension(string_view(".txt")), ".txt");
        DOCTEST_CHECK_EQ(extension(string_view("/foo/bar")), "");
    }

    DOCTEST_TEST_CASE("changeExtension") {
        DOCTEST_CHECK_EQ(changeExtension("", ".json"), ".json");
        DOCTEST_CHECK_EQ(changeExtension("/foo/bar.txt", ".json"), "/foo/bar.json");
        DOCTEST_CHECK_EQ(changeExtension(".txt", ".json"), ".json");
        DOCTEST_CHECK_EQ(changeExtension("/foo/bar", ".json"), "/foo/bar.json");
    }

    DOCTEST_TEST_CASE("filename") {
        DOCTEST_CHECK_EQ(filename(""), "");
        DOCTEST_CHECK_EQ(filename("/foo/bar.txt"), "bar.txt");
        DOCTEST_CHECK_EQ(filename(".txt"), ".txt");
        DOCTEST_CHECK_EQ(filename("/foo/bar"), "bar");
    }

    DOCTEST_TEST_CASE("filebasename") {
        DOCTEST_CHECK_EQ(filebasename(""), "");
        DOCTEST_CHECK_EQ(filebasename("/foo/bar.txt"), "bar");
        DOCTEST_CHECK_EQ(filebasename(".txt"), "");
        DOCTEST_CHECK_EQ(filebasename("/foo/bar"), "bar");
    }

    DOCTEST_TEST_CASE("parent") {
        DOCTEST_CHECK_EQ(parent(""), "");
        DOCTEST_CHECK_EQ(parent("/foo/bar.txt"), "/foo");
        DOCTEST_CHECK_EQ(parent("bar.txt"), "/");
        DOCTEST_CHECK_EQ(parent("/foo/bar"), "/foo");
    }

    DOCTEST_TEST_CASE("isNormalized") {
        DOCTEST_CHECK_EQ(isNormalized("/foo/bar.txt"), true);
        DOCTEST_CHECK_EQ(isNormalized("/foo"), true);
        DOCTEST_CHECK_EQ(isNormalized("/bar.txt"), true);

        DOCTEST_CHECK_EQ(isNormalized(""), false);
        DOCTEST_CHECK_EQ(isNormalized("/foo\\bar.txt"), false);
        DOCTEST_CHECK_EQ(isNormalized("//foo/bar.txt"), false);
        DOCTEST_CHECK_EQ(isNormalized("/foo/.bar.txt"), false);
        DOCTEST_CHECK_EQ(isNormalized("/foo/bar..txt"), false);
        DOCTEST_CHECK_EQ(isNormalized("bar.txt"), false);
        DOCTEST_CHECK_EQ(isNormalized("/foo/bar/"), false);
        DOCTEST_CHECK_EQ(isNormalized("/foo/bar."), false);
        DOCTEST_CHECK_EQ(isNormalized("/foo./bar.txt"), false);
    }

    DOCTEST_TEST_CASE("normalize") {
        DOCTEST_CHECK_EQ(normalize("/foo/bar.txt"), "/foo/bar.txt");
        DOCTEST_CHECK_EQ(normalize("/foo"), "/foo");
        DOCTEST_CHECK_EQ(normalize("/bar.txt"), "/bar.txt");

        DOCTEST_CHECK_EQ(normalize(""), "/");
        DOCTEST_CHECK_EQ(normalize("/foo\\bar.txt"), "/foo/bar.txt");
        DOCTEST_CHECK_EQ(normalize("//foo/bar.txt"), "/foo/bar.txt");
        DOCTEST_CHECK_EQ(normalize("/foo/bar.txt"), "/foo/bar.txt");
        DOCTEST_CHECK_EQ(normalize("bar.txt"), "/bar.txt");
        DOCTEST_CHECK_EQ(normalize("/foo/bar/"), "/foo/bar");

        DOCTEST_CHECK_EQ(normalize("/foo/../bar/"), "/bar");
        DOCTEST_CHECK_EQ(normalize("/foo/bar/.."), "/foo");
        DOCTEST_CHECK_EQ(normalize("/foo/.."), "/");

        DOCTEST_CHECK_EQ(normalize("/foo/./bar/"), "/foo/bar");
        DOCTEST_CHECK_EQ(normalize("/foo/bar/."), "/foo/bar");
        DOCTEST_CHECK_EQ(normalize("/foo/."), "/foo");
        DOCTEST_CHECK_EQ(normalize("./foo"), "/foo");
        DOCTEST_CHECK_EQ(normalize("./foo/../bar/./../baz/."), "/baz");
        DOCTEST_CHECK_EQ(normalize("/foo/../.."), "/");
        DOCTEST_CHECK_EQ(normalize("/foo/../../bar"), "/bar");
        DOCTEST_CHECK_EQ(normalize("."), "/");
        DOCTEST_CHECK_EQ(normalize(".."), "/");
    }

    DOCTEST_TEST_CASE("join") {
        DOCTEST_CHECK_EQ(join({"/foo", "bar.txt"}), "/foo/bar.txt");
        DOCTEST_CHECK_EQ(join({"/foo", "/bar", "baz.txt"}), "/foo//bar/baz.txt");
        DOCTEST_CHECK_EQ(join({"/foo", "", "/bar.txt"}), "/foo//bar.txt");
        DOCTEST_CHECK_EQ(join({"", "foo", "/bar.txt"}), "foo//bar.txt");
        DOCTEST_CHECK_EQ(join({"foo", "bar", ""}), "foo/bar");
        DOCTEST_CHECK_EQ(join({}), "");

        DOCTEST_CHECK_EQ(join("/foo", "/bar"_sv, "baz.txt"_s), "/foo//bar/baz.txt");
    }
}
