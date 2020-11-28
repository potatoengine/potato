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
    };
} // namespace up
