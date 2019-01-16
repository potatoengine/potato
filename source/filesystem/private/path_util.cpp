// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/path_util.h"
#include "grimm/foundation/assertion.h"

// returns extension, including dot, e.g. foo.txt -> .txt
// only the last extension is returned, e.g. foo.txt.gz -> .gz
gm::zstring_view gm::fs::path::extension(zstring_view path) noexcept {
    auto pos = path.find_last_of("/\\.");
    if (pos != zstring_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

gm::string_view gm::fs::path::extension(string_view path) noexcept {
    auto pos = path.find_last_of("/\\.");
    if (pos != string_view::npos && path[pos] == '.') {
        return path.substr(pos);
    }
    return {};
}

// extension must include the dot, e.g. .txt
std::string gm::fs::path::changeExtension(string_view path, string_view extension) {

    GM_ASSERT(extension.empty() || extension.front() == '.');

    auto pos = path.find_last_of("/\\.");
    if (pos == string_view::npos || path[pos] != '.') {
        pos = path.size();
    }

    std::string result;
    result.resize(pos + extension.size());
    std::memcpy(result.data(), path.data(), pos);
    std::memcpy(result.data() + pos, extension.data(), extension.size());
    return result;
}

// returns the filename of a path, e.g. foo/bar.txt -> bar.txt
gm::string_view gm::fs::path::filename(string_view path) noexcept {
    auto pos = path.find_last_of("/\\");
    if (pos != string_view::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

// returns the basename of a path, e.g. foo/bar.txt -> bar
gm::string_view gm::fs::path::filebasename(string_view path) noexcept {
    auto ext = extension(path);
    auto pathWithoutExt = path.substr(0, path.size() - ext.size());
    auto pos = pathWithoutExt.find_last_of("/\\");
    if (pos != string_view::npos) {
        return pathWithoutExt.substr(pos + 1);
    }
    return pathWithoutExt;
}

gm::string_view gm::fs::path::parent(string_view path) noexcept {
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

bool gm::fs::path::isNormalized(string_view path) noexcept {
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

std::string gm::fs::path::normalize(string_view path) {
    if (path.empty()) {
        return "/";
    }

    std::string result;
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
                result.push_back(ch);
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
                result.push_back('/');
                result.push_back(ch);
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
                result.push_back('.');
                result.push_back(ch);
                mode = Component;
            }
            break;
        }
    }

    if (result.empty()) {
        result.push_back('/');
    }

    return result;
}
