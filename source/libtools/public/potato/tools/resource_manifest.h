// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"

#include "potato/spud/int_types.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up {
    using ResourceId = uint64;
    class Stream;

    /// @brief Mapping of resource identifiers to CAS hashes and filenames
    class ResourceManifest {
    public:
        static constexpr int version = 1;

        UP_TOOLS_API uint64 findHash(ResourceId id) const noexcept;
        UP_TOOLS_API zstring_view findFilename(ResourceId id) const noexcept;

        UP_TOOLS_API bool parseManifest(Stream& stream);
        UP_TOOLS_API bool writeManifest(Stream& stream) const;

        UP_TOOLS_API void addRecord(ResourceId id, uint64 hash, string filename);

    private:
        struct Record {
            ResourceId id = {};
            uint64 hash = 0;
            string filename;
        };

        vector<Record> _records;
    };
} // namespace up
