// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/asset_loader.h"
#include "potato/spud/rc.h"

namespace up {
    class SoundResource : public Asset {
    public:
        using Asset::Asset;
        static constexpr zstring_view assetTypeName = "potato.asset.sound"_zsv;

    protected:
    };
} // namespace up
