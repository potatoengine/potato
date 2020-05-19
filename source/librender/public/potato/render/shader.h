// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/rc.h"
#include "potato/spud/vector.h"
#include "potato/spud/int_types.h"

namespace up {
    class RenderContext;

    class Shader : public shared<Shader> {
    public:
        explicit Shader(vector<byte> shader) : _content(std::move(shader)) {}
        ~Shader() = default;

        view<byte> content() const noexcept { return _content; }

    private:
        vector<byte> _content;
    };
} // namespace up
