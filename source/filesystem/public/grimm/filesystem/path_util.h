// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/zstring_view.h"

namespace gm::fs::path {
    static constexpr std::size_t maxPathLength = 4096;

    // returns extension, including dot, e.g. foo.txt -> .txt
    // only the last extension is returned, e.g. foo.txt.gz -> .gz
    GM_FILESYSTEM_API zstring_view extension(zstring_view path) noexcept;
    GM_FILESYSTEM_API string_view extension(string_view path) noexcept;

    // replaces any existing extension with the requested one
    // extension must include the dot, e.g. .txt
    // if the path has no extension, adds the provided one, e.g. -> foo/bar -> foo/bar.txt
    GM_FILESYSTEM_API std::string changeExtension(string_view path, string_view extension);

    // returns the filename of a path, e.g. foo/bar.txt -> bar.txt
    GM_FILESYSTEM_API string_view filename(string_view path) noexcept;

    // returns the basename of a path, e.g. foo/bar.txt -> bar
    GM_FILESYSTEM_API string_view basename(string_view path) noexcept;

    // returns the parent of a path, e.g. foo/bar.txt -> foo
    // if the path is empty, returns the empty string
    // if the path is only a /, returns /
    // if the path contains no directory separator, returns /
    GM_FILESYSTEM_API string_view parent(string_view path) noexcept;

    // returns true if the path has only forward slashes, no repeat slashes, no leading dots,
    // no duplicate dots or duplicate slashes, non-empty, starts with leading slash,
    // no trailing slash or trailing dot in component
    GM_FILESYSTEM_API bool isNormalized(string_view path) noexcept;

    // converts a path so that isNormalized is true
    // empty path is returned as /
    // leading slash added to path, e.g. foo.txt -> /foo.txt
    // trailing slash is stripped, e.g. /foo/ -> /foo
    // duplicate slashes or dots are condensed, e.g. /foo//bar..txt -> /foo/bar.txt
    // leading or trailing dots in components are stripped, e.g. /.foo/bar. -> /foo/bar
    GM_FILESYSTEM_API std::string normalize(string_view path);
} // namespace gm::fs::path
