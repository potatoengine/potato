// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"

namespace gm::gpu {
    class Buffer;
    class CommandList;
    class Device;
} // namespace gm::gpu

namespace gm {
    class Material;
    class Mesh;
    class RenderContext;

    class Model {
    public:
        GM_RENDER_API explicit Model(rc<Material> material);
        GM_RENDER_API ~Model();

        GM_RENDER_API void render(RenderContext& ctx);

    private:
        rc<Material> _material;
        rc<Mesh> _mesh;
    };
} // namespace gm
