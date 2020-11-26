// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/zstring_view.h"

namespace up {
    class AssetEditService {
    public:
        UP_EDITOR_API char8_t const* getIconForType(zstring_view type) const noexcept;
        UP_EDITOR_API zstring_view getEditorForType(zstring_view type) const noexcept;
    };
} // namespace up
