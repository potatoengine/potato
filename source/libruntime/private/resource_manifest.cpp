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
    int rootIdColumn = -1;
    int logicalIdColumn = -1;
    int logicalNameColumn = -1;
    int contentHashColumn = -1;
    int debugNameColumn = -1;

    enum ColumnMask {
        ColumnRootIdMask = (1 << 0),
        ColumnLogicalIdMask = (1 << 1),
        ColumnLogicalNameMask = (1 << 2),
        ColumnContentHashMask = (1 << 3),
        ColumnDebugNameMask = (1 << 4),
        ColumnRequiredMask = ColumnRootIdMask | ColumnLogicalIdMask | ColumnLogicalNameMask | ColumnContentHashMask
    };

    string_view::size_type sep = 0;
    int column = 0;
    bool eol = false;
    int mask = 0;
    Record record;

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
                input.pop_front();
                if (auto const eol = input.find('\n'); eol != string_view::npos) {
                    input = input.substr(eol + 1);
                }
                else {
                    input = {};
                }
                break;
            case ':': // header
                input.pop_front();
                sep = 0;
                column = 0;
                mask = 0;
                eol = false;
                while (!eol && (sep = input.find_first_of("|\n")) != string_view::npos) {
                    string_view const header = input.substr(0, sep);

                    if (header == columnRootId) {
                        rootIdColumn = column;
                        mask |= ColumnRootIdMask;
                    }
                    else if (header == columnLogicalId) {
                        logicalIdColumn = column;
                        mask |= ColumnLogicalIdMask;
                    }
                    else if (header == columnLogicalName) {
                        logicalNameColumn = column;
                        mask |= ColumnLogicalNameMask;
                    }
                    else if (header == columnContentHash) {
                        contentHashColumn = column;
                        mask |= ColumnContentHashMask;
                    }
                    else if (header == columnDebugName) {
                        debugNameColumn = column;
                        mask |= ColumnDebugNameMask;
                    }

                    ++column;
                    eol = input[sep] == '\n';
                    input = input.substr(sep + 1);
                }

                if ((mask & ColumnRequiredMask) != ColumnRequiredMask) {
                    return false;
                }
                break;
            default: // content
                sep = 0;
                column = 0;
                mask = 0;
                record = {};
                eol = false;
                while (!eol && (sep = input.find_first_of("|\n")) != string_view::npos) {
                    string_view const data = input.substr(0, sep);

                    if (column == rootIdColumn) {
                        std::from_chars(data.begin(), data.end(), static_cast<uint64&>(record.rootId), 16);
                        mask |= ColumnRootIdMask;
                    }
                    else if (column == logicalIdColumn) {
                        std::from_chars(data.begin(), data.end(), static_cast<uint64&>(record.logicalId), 16);
                        mask |= ColumnLogicalIdMask;
                    }
                    else if (column == logicalNameColumn) {
                        std::from_chars(data.begin(), data.end(), static_cast<uint64&>(record.logicalName), 16);
                        mask |= ColumnLogicalNameMask;
                    }
                    else if (column == contentHashColumn) {
                        std::from_chars(data.begin(), data.end(), record.hash, 16);
                        mask |= ColumnContentHashMask;
                    }
                    else if (column == debugNameColumn) {
                        record.filename = data;
                        mask |= ColumnDebugNameMask;
                    }

                    ++column;
                    eol = input[sep] == '\n';
                    input = input.substr(sep + 1);
                }

                if ((mask & ColumnRequiredMask) != ColumnRequiredMask) {
                    return false;
                }

                manifest._records.push_back(std::move(record));
                break;
        }
    }

    return true;
}
