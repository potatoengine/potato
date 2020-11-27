// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/resource_loader.h"
#include "potato/spud/rc.h"

namespace up {
    class SoundResource : public Resource {
    public:
        static constexpr zstring_view resourceType = "potato.asset.sound"_zsv;

        virtual ~SoundResource() = default;
    };
} // namespace up
