// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Project {
    public:
        [[nodiscard]] UP_TOOLS_API static box<Project> loadFromFile(zstring_view filename);

        auto targetPath() noexcept -> zstring_view { return _targetPath; }

    private:
        string _assetRootPath;
        string _targetPath;
        string _cachePath;
    };
} // namespace up
