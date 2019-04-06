// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/vector.h"
#include <glm/mat4x4.hpp>

namespace up::gpu {
    class CommandList;
    class Device;
} // namespace up::gpu

namespace up {
    class Model;
    class RenderContext;

    class Node {
    public:
        UP_RENDER_API explicit Node(box<Model> model);
        UP_RENDER_API ~Node();

        UP_RENDER_API void render(RenderContext& ctx);
        UP_RENDER_API void addChild(box<Node> child);

        glm::mat4x4 UP_VECTORCALL transform() const { return _transform; }
        void UP_VECTORCALL transform(glm::mat4x4 transform) { _transform = transform; }

    private:
        glm::mat4x4 _transform;
        box<Model> _model;
        vector<box<Node>> _children;
    };
} // namespace up