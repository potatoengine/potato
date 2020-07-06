// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

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
        };

        static constexpr int version = 1;

        auto size() const noexcept { return _records.size(); }

        view<Record> records() const noexcept { return _records; }

        UP_RUNTIME_API uint64 findHash(ResourceId id) const noexcept;
        UP_RUNTIME_API zstring_view findFilename(ResourceId id) const noexcept;

        static UP_RUNTIME_API bool parseManifest(string_view input, ResourceManifest& manifest);

    private:
        vector<Record> _records;
    };
} // namespace up
