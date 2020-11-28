// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/format/erased.h"
#include "potato/spud/int_types.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up {
    using ResourceId = uint64;

    /// @brief Mapping of resource identifiers to CAS hashes and filenames
    class ResourceManifest {
    public:
        struct Record {
            ResourceId rootId = {};
            ResourceId logicalId = {};
            uint64 logicalName = 0;
            uint64 hash = 0;
            string filename;
            string type;
        };

        static constexpr zstring_view columnRootId = "ROOT_ID"_zsv;
        static constexpr zstring_view columnLogicalId = "LOGICAL_ID"_zsv;
        static constexpr zstring_view columnLogicalName = "LOGICAL_NAME"_zsv;
        static constexpr zstring_view columnContentType = "CONTENT_TYPE"_zsv;
        static constexpr zstring_view columnContentHash = "CONTENT_HASH"_zsv;
        static constexpr zstring_view columnDebugName = "DEBUG_NAME"_zsv;
        static constexpr int version = 2;

        void clear() { _records.clear(); }
        auto size() const noexcept { return _records.size(); }

        view<Record> records() const noexcept { return _records; }

        UP_RUNTIME_API uint64 findHash(ResourceId id) const noexcept;
        UP_RUNTIME_API zstring_view findFilename(ResourceId id) const noexcept;

        static UP_RUNTIME_API bool parseManifest(string_view input, ResourceManifest& manifest);

    private:
        vector<Record> _records;
    };
} // namespace up
