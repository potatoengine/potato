// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/asset_loader.h"
#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"
#include "potato/spud/vector.h"

namespace up {
    class RenderContext;

    class Shader : public Asset {
    public:
        static constexpr zstring_view assetTypeName = "potato.asset.shader"_zsv;

        explicit Shader(vector<byte> shader) : _content(std::move(shader)) {}

        view<byte> content() const noexcept { return _content; }

    private:
        vector<byte> _content;
    };
} // namespace up
