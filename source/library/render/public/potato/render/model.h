// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/platform.h"
#include <glm/mat4x4.hpp>

namespace up::gpu {
    class GpuBuffer;
    class CommandList;
    class Device;
} // namespace up::gpu

namespace up {
    class Material;
    class Mesh;
    class RenderContext;

    class Model {
    public:
        UP_RENDER_API explicit Model(rc<Mesh> mesh, rc<Material> material);
        UP_RENDER_API ~Model();

        UP_RENDER_API void UP_VECTORCALL render(RenderContext& ctx, glm::mat4x4 transform);

    private:
        rc<Material> _material;
        rc<Mesh> _mesh;
        box<gpu::GpuBuffer> _transformBuffer;
    };
} // namespace up
