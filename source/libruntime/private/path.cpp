// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/runtime/path.h"
#include <potato/runtime/assertion.h>
#include "potato/spud/string_writer.h"

// returns extension, including dot, e.g. foo.txt -> .txt
// only the last extension is returned, e.g. foo.txt.gz -> .gz
up::zstring_view up::path::extension(zstring_view path) noexcept {
    auto const pos = path.find_last_of("/\\.");
    if (pos != zstring_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

up::string_view up::path::extension(string_view path) noexcept {
    auto const pos = path.find_last_of("/\\.");
    if (pos != string_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

// extension must include the dot, e.g. .txt
auto up::path::changeExtension(string_view path, string_view extension) -> string {

    UP_ASSERT(extension.empty() || extension.front() == '.');

    auto const sepPos = path.find_last_of("/\\.");
    auto const pos = sepPos == string_view::npos || path[sepPos] != '.' ? path.size() : sepPos;

    string_writer result;
    result.reserve(pos + extension.size());
    result.append(path.data(), pos);
    result.append(extension.data(), extension.size());
    return string(result);
}

// returns the filename of a path, e.g. foo/bar.txt -> bar.txt
up::string_view up::path::filename(string_view path) noexcept {
    auto const pos = path.find_last_of("/\\");
    if (pos != string_view::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

// returns the basename of a path, e.g. foo/bar.txt -> bar
up::string_view up::path::filebasename(string_view path) noexcept {
    auto const ext = extension(path);
    auto const pathWithoutExt = path.substr(0, path.size() - ext.size());
    auto const pos = pathWithoutExt.find_last_of("/\\");
    if (pos != string_view::npos) {
        return pathWithoutExt.substr(pos + 1);
    }
    return pathWithoutExt;
}

up::string_view up::path::parent(string_view path) noexcept {
    if (path.empty()) {
        return path;
    }
    auto const pos = path.find_last_of("/\\");
    if (pos == 0) {
        return path.substr(0, 1);
    }
    if (pos != string_view::npos) {
        return path.substr(0, pos);
    }
    return "/";
}

bool up::path::isNormalized(string_view path) noexcept {
    // ensure path starts with a /
    if (path.empty() || path.front() != '/') {
        return false;
    }

    enum class Part {
        Component,
        Slash,
        Dot
    } mode = Part::Slash;

    for (auto const ch : path.substr(1)) {
        if (ch == '\\') {
            // no back-slashes allowed
            return false;
        }

        switch (mode) {
        case Part::Component:
            if (ch == '/') {
                mode = Part::Slash;
            }
            else if (ch == '.') {
                mode = Part::Dot;
            }
            break;
        case Part::Slash:
            if (ch == '/') {
                // no duplicate slashes allowed
                return false;
            }
            else if (ch == '.') {
                // no leading dots allowed
                return false;
            }
            mode = Part::Component;
            break;
        case Part::Dot:
            if (ch == '.') {
                // no duplicate dots allowed
                return false;
            }
            else if (ch == '/') {
                // no trailing dots allowed
                return false;
            }
            else {
                mode = Part::Component;
            }
            break;
        }
    }

    // no trailing slash or dot
    return mode != Part::Component;
}

auto up::path::normalize(string_view path) -> string {
    if (path.empty()) {
        return string("/");
    }

    string_writer result;
    result.reserve(path.size());

    enum class Part {
        Component,
        Slash,
        Dot
    } mode = Part::Slash;

    for (auto const ch : path) {
        switch (mode) {
        case Part::Component:
            if (ch == '/' || ch == '\\') {
                mode = Part::Slash;
            }
            else if (ch == '.') {
                mode = Part::Dot;
            }
            else {
                result.append(ch);
            }
            break;
        case Part::Slash:
            if (ch == '/' || ch == '\\') {
                // ignore duplicate slash
                break;
            }
            else if (ch == '.') {
                // ignore leading dots
                break;
            }
            else {
                result.append('/');
                result.append(ch);
                mode = Part::Component;
            }
            break;
        case Part::Dot:
            if (ch == '.') {
                // ignore duplicate dots
                break;
            }
            else if (ch == '/' || ch == '\\') {
                // ignore trailing dots
                mode = Part::Slash;
            }
            else {
                result.append('.');
                result.append(ch);
                mode = Part::Component;
            }
            break;
        }
    }

    if (result.empty()) {
        result.append('/');
    }

    return string(result);
}

auto up::path::join(std::initializer_list<string_view> components) -> string {
    std::size_t size = 0;

    for (auto sv : components) {
        // path separator
        if (!sv.empty() && size != 0) {
            ++size;
        }
        size += sv.size();
    }

    string_writer result;
    result.reserve(size);

    for (auto sv : components) {
        if (!sv.empty() && !result.empty()) {
            result.append('/');
        }
        result.append(sv.data(), sv.size());
    }

    return std::move(result).to_string();
}
