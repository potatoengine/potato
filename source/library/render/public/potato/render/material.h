// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/vector.h"

namespace up::gpu {
    class CommandList;
    class GpuDevice;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
} // namespace up::gpu

namespace up {
    class RenderContext;
    class Shader;
    class Texture;

    class Material : public shared<Material> {
    public:
        UP_RENDER_API explicit Material(rc<Shader> vertexShader, rc<Shader> pixelShader, vector<rc<Texture>> textures);
        UP_RENDER_API ~Material();

        UP_RENDER_API void bindMaterialToRender(RenderContext& ctx);

    private:
        box<gpu::GpuPipelineState> _state;
        rc<Shader> _vertexShader;
        rc<Shader> _pixelShader;
        vector<rc<Texture>> _textures;
        vector<box<gpu::GpuResourceView>> _srvs;
        vector<box<gpu::GpuSampler>> _samplers;
    };
} // namespace up
