// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/platform.h"
#include <glm/mat4x4.hpp>

namespace up {
    class GpuBuffer;
    class CommandList;
    class GpuDevice;
} // namespace up

namespace up {
    class Material;
    class Mesh;
    class RenderContext;

    class Model : public shared<Model> {
    public:
        UP_RENDER_API explicit Model(rc<Mesh> mesh, rc<Material> material);
        UP_RENDER_API ~Model();

        UP_RENDER_API void UP_VECTORCALL render(RenderContext& ctx, glm::mat4x4 transform);

    private:
        rc<Material> _material;
        rc<Mesh> _mesh;
        box<GpuBuffer> _transformBuffer;
    };
} // namespace up
