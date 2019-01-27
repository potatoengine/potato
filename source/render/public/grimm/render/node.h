// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"

namespace gm::gpu {
    class CommandList;
    class Device;
} // namespace gm::gpu

namespace gm {
    class Camera;
    class Model;

    class Node {
    public:
        GM_RENDER_API explicit Node(box<Camera> camera);
        GM_RENDER_API explicit Node(box<Model> model);
        GM_RENDER_API ~Node();

        GM_RENDER_API void render(gpu::CommandList& commandList, gpu::Device& device);

    private:
        box<Camera> _camera;
        box<Model> _model;
    };
} // namespace gm
