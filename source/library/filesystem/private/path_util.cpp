// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/path_util.h"
#include "potato/foundation/assertion.h"
#include "potato/foundation/string_writer.h"

// returns extension, including dot, e.g. foo.txt -> .txt
// only the last extension is returned, e.g. foo.txt.gz -> .gz
up::zstring_view up::path::extension(zstring_view path) noexcept {
    auto pos = path.find_last_of("/\\.");
    if (pos != zstring_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

up::string_view up::path::extension(string_view path) noexcept {
    auto pos = path.find_last_of("/\\.");
    if (pos != string_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

// extension must include the dot, e.g. .txt
auto up::path::changeExtension(string_view path, string_view extension) -> string {

    UP_ASSERT(extension.empty() || extension.front() == '.');

    auto pos = path.find_last_of("/\\.");
    if (pos == string_view::npos || path[pos] != '.') {
        pos = path.size();
    }

    string_writer result;
    result.reserve(pos + extension.size());
    result.write(path.data(), pos);
    result.write(extension.data(), extension.size());
    return string(result);
}

// returns the filename of a path, e.g. foo/bar.txt -> bar.txt
up::string_view up::path::filename(string_view path) noexcept {
    auto pos = path.find_last_of("/\\");
    if (pos != string_view::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

// returns the basename of a path, e.g. foo/bar.txt -> bar
up::string_view up::path::filebasename(string_view path) noexcept {
    auto ext = extension(path);
    auto pathWithoutExt = path.substr(0, path.size() - ext.size());
    auto pos = pathWithoutExt.find_last_of("/\\");
    if (pos != string_view::npos) {
        return pathWithoutExt.substr(pos + 1);
    }
    return pathWithoutExt;
}

up::string_view up::path::parent(string_view path) noexcept {
    if (path.empty()) {
        return path;
    }
    auto pos = path.find_last_of("/\\");
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

    enum {
        Component,
        Slash,
        Dot
    } mode = Slash;

    for (auto ch : path.substr(1)) {
        if (ch == '\\') {
            // no back-slashes allowed
            return false;
        }

        switch (mode) {
        case Component:
            if (ch == '/') {
                mode = Slash;
            }
            else if (ch == '.') {
                mode = Dot;
            }
            break;
        case Slash:
            if (ch == '/') {
                // no duplicate slashes allowed
                return false;
            }
            else if (ch == '.') {
                // no leading dots allowed
                return false;
            }
            mode = Component;
            break;
        case Dot:
            if (ch == '.') {
                // no duplicate dots allowed
                return false;
            }
            else if (ch == '/') {
                // no trailing dots allowed
                return false;
            }
            else {
                mode = Component;
            }
            break;
        }
    }

    if (mode != Component) {
        // no trailing slash or dot
        return false;
    }

    return true;
}

auto up::path::normalize(string_view path) -> string {
    if (path.empty()) {
        return string("/");
    }

    string_writer result;
    result.reserve(path.size());

    enum {
        Component,
        Slash,
        Dot
    } mode = Slash;

    for (auto ch : path) {
        switch (mode) {
        case Component:
            if (ch == '/' || ch == '\\') {
                mode = Slash;
            }
            else if (ch == '.') {
                mode = Dot;
            }
            else {
                result.write(ch);
            }
            break;
        case Slash:
            if (ch == '/' || ch == '\\') {
                // ignore duplicate slash
                break;
            }
            else if (ch == '.') {
                // ignore leading dots
                break;
            }
            else {
                result.write('/');
                result.write(ch);
                mode = Component;
            }
            break;
        case Dot:
            if (ch == '.') {
                // ignore duplicate dots
                break;
            }
            else if (ch == '/' || ch == '\\') {
                // ignore trailing dots
                mode = Slash;
            }
            else {
                result.write('.');
                result.write(ch);
                mode = Component;
            }
            break;
        }
    }

    if (result.empty()) {
        result.write('/');
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
            result.write('/');
        }
        result.write(sv.data(), sv.size());
    }

    return std::move(result).to_string();
}
