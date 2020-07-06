// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "resource_manifest.h"
#include "stream.h"

#include "potato/format/format.h"

#include <charconv>

auto up::ResourceManifest::findHash(ResourceId id) const noexcept -> uint64 {
    for (auto const& record : _records) {
        if (record.logicalId == id) {
            return record.hash;
        }
    }

    return 0;
}

auto up::ResourceManifest::findFilename(ResourceId id) const noexcept -> zstring_view {
    for (auto const& record : _records) {
        if (record.logicalId == id) {
            return record.filename;
        }
    }

    return {};
}

bool up::ResourceManifest::parseManifest(string_view input, ResourceManifest& manifest) {
    while (!input.empty()) {
        switch (input.front()) {
        case '#': // comment
            if (auto const eol = input.find('\n'); eol != string_view::npos) {
                input = input.substr(eol + 1);
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
                input = input.substr(eol + 1);
            }
            else {
                input = {};
            }
            break;
        case ':': // header
            if (auto const eol = input.find('\n'); eol != string_view::npos) {
                input = input.substr(eol + 1);
            }
            else {
                input = {};
            }
            break;
        default: { // content
            string_view::size_type sep = {};
            bool eol = false;
            int column = 0;
            Record record;
            while (!eol && (sep = input.find_first_of("|\n")) != string_view::npos) {
                switch (column++) {
                case 0: std::from_chars(input.data(), input.data() + sep, static_cast<uint64&>(record.rootId), 16); break;
                case 1: std::from_chars(input.data(), input.data() + sep, static_cast<uint64&>(record.logicalId), 16); break;
                case 2: std::from_chars(input.data(), input.data() + sep, static_cast<uint64&>(record.logicalName), 16); break;
                case 3: std::from_chars(input.data(), input.data() + sep, record.hash, 16); break;
                case 4: record.filename = input.substr(0, sep); break;
                default: break;
                }

                eol = input[sep] == '\n';
                input = input.substr(sep + 1);
            }

            manifest._records.push_back(std::move(record));
        } break;
        }
    }

    return true;
}
