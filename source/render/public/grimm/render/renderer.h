// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

namespace gm {
    class Renderer {
    public:
        GM_RENDER_API Renderer();
        GM_RENDER_API ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;
    };
} // namespace gm
