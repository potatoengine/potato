// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/resource_view.h"

namespace gm {
    class ResourceViewD3D11 final : public GpuResourceView {
    public:
        enum class Type {
            RTV,
            UAV,
            DSV
        };

        explicit ResourceViewD3D11(Type type, com_ptr<ID3D11View> view);
        virtual ~ResourceViewD3D11();

        ResourceViewD3D11(ResourceViewD3D11&&) = delete;
        ResourceViewD3D11& operator=(ResourceViewD3D11&&) = delete;

        Type getType() const { return _type; }
        com_ptr<ID3D11View> const& getView() const { return _view; }

    private:
        Type _type;
        com_ptr<ID3D11View> _view;
    };
} // namespace gm
