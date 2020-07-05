// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "resource_manifest.h"

#include "potato/format/format.h"
#include "potato/runtime/stream.h"

auto up::ResourceManifest::findHash(ResourceId id) const noexcept -> uint64 {
    for (auto const& record : _records) {
        if (record.id == id) {
            return record.hash;
        }
    }

    return 0;
}

auto up::ResourceManifest::findFilename(ResourceId id) const noexcept -> zstring_view {
    for (auto const& record : _records) {
        if (record.id == id) {
            return record.filename;
        }
    }

    return {};
}

bool up::ResourceManifest::parseManifest(string_view input) {
    while (!input.empty()) {
        switch (input.front()) {
        case '#': // comment
            if (auto const eol = input.find('\n'); eol != string_view::npos) {
                input = input.substr(eol);
            }
            else {
                input = {};
            }
            break;
        case '\n': // blank line
            input.pop_front();
            break;
        case '.': // metadata
            if (auto const eol = input.find('\n'); eol != string_view::npos) {
                input = input.substr(eol);
            }
            else {
                input = {};
            }
            break;
        default
            : // content line
        {
            auto const sep = input.find_first_of("|\n");
            bool eol = false;
            while (sep != string_view::npos && !eol) {
                eol = input[sep] == '\n';
                input = input.substr(sep);
            }
        } break;
        }
    }

    return true;
}

bool up::ResourceManifest::writeManifest(erased_writer writer) const {
    format_to(writer,
        "# Potato Manifest\n"
        ".version={}\n"
        ":ID|HASH|NAME\n",
        version);

    for (auto const& record : _records) {
        format_to(writer, "{}|{}|{}\n", record.id, record.hash, record.filename);
    }

    return true;
}

void up::ResourceManifest::addRecord(ResourceId id, uint64 hash, string filename) {
    for (auto& record : _records) {
        if (record.id == id) {
            record.hash = hash;
            record.filename = std::move(filename);
            return;
        }
    }

    _records.push_back({id, hash, std::move(filename)});
}
