// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include "grimm/foundation/vector.h"

namespace gm::gpu {
    class CommandList;
    class Device;
    class PipelineState;
    class ResourceView;
    class Sampler;
} // namespace gm::gpu

namespace gm {
    class RenderContext;
    class Shader;
    class Texture;

    class Material : public shared<Material> {
    public:
        GM_RENDER_API explicit Material(rc<Shader> vertexShader, rc<Shader> pixelShader, vector<rc<Texture>> textures);
        GM_RENDER_API ~Material();

        GM_RENDER_API void bindMaterialToRender(RenderContext& ctx);

    private:
        box<gpu::PipelineState> _state;
        rc<Shader> _vertexShader;
        rc<Shader> _pixelShader;
        vector<rc<Texture>> _textures;
        vector<box<gpu::ResourceView>> _srvs;
        vector<box<gpu::Sampler>> _samplers;
    };
} // namespace gm
