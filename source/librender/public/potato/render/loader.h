// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/spud/zstring_view.h"
#include "potato/spud/rc.h"

namespace up {
    class Mesh;
    class Material;
    class Shader;
    class Texture;

    class Loader {
    public:
        virtual auto loadMeshSync(zstring_view path) -> rc<Mesh> = 0;
        virtual auto loadMaterialSync(zstring_view path) -> rc<Material> = 0;
        virtual auto loadShaderSync(zstring_view path) -> rc<Shader> = 0;
        virtual auto loadTextureSync(zstring_view path) -> rc<Texture> = 0;

    protected:
        Loader() = default;
        ~Loader() = default;
    };
}
