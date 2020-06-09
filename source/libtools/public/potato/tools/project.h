// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "_export.h"

#include "potato/spud/box.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Project {
    public:
        [[nodiscard]] UP_TOOLS_API static box<Project> loadFromFile(zstring_view filename);
    };
} // namespace up
