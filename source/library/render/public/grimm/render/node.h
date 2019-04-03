// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/vector.h"
#include <glm/mat4x4.hpp>

namespace gm::gpu {
    class CommandList;
    class Device;
} // namespace gm::gpu

namespace gm {
    class Model;
    class RenderContext;

    class Node {
    public:
        GM_RENDER_API explicit Node(box<Model> model);
        GM_RENDER_API ~Node();

        GM_RENDER_API void render(RenderContext& ctx);
        GM_RENDER_API void addChild(box<Node> child);

        glm::mat4x4 GM_VECTORCALL transform() const { return _transform; }
        void GM_VECTORCALL transform(glm::mat4x4 transform) { _transform = transform; }

    private:
        glm::mat4x4 _transform;
        box<Model> _model;
        vector<box<Node>> _children;
    };
} // namespace gm
