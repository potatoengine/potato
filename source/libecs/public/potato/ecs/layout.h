// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "common.h"

namespace up {
    /// @brief Describes the information about how components are laid out in an Archetype
    ///
    struct LayoutRow {
        ComponentId component = ComponentId::Unknown;
        ComponentMeta const* meta = nullptr;
        uint16 offset = 0;
        uint16 width = 0;
    };
} // namespace up
