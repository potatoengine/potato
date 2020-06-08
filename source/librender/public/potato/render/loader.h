// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class Mesh;
    class Material;
    class Shader;
    class Texture;

    class Loader {
    public:
        virtual ~Loader() = default;

        virtual auto loadMeshSync(zstring_view path) -> rc<Mesh> = 0;
        virtual auto loadMaterialSync(zstring_view path) -> rc<Material> = 0;
        virtual auto loadShaderSync(zstring_view path) -> rc<Shader> = 0;
        virtual auto loadTextureSync(zstring_view path) -> rc<Texture> = 0;
    };
} // namespace up
