// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/rc.h"
#include "grimm/foundation/blob.h"

namespace gm {
    class RenderContext;

    class Shader : public shared<Shader> {
    public:
        explicit Shader(blob shader) : _content(std::move(shader)) {}
        ~Shader() = default;

        view<byte> content() const noexcept { return _content; }

    private:
        blob _content;
    };
} // namespace gm
