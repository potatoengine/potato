// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/vector.h"

namespace gm::gpu {
    class CommandList;
    class Device;
} // namespace gm::gpu

namespace gm {
    class Model;

    class Node {
    public:
        GM_RENDER_API explicit Node(box<Model> model);
        GM_RENDER_API ~Node();

        GM_RENDER_API void render(gpu::CommandList& commandList, gpu::Device& device);
        GM_RENDER_API void addChild(box<Node> child);

    private:
        box<Model> _model;
        vector<box<Node>> _children;
    };
} // namespace gm
