// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "grimm/gpu/com_ptr.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/resource_view.h"

namespace gm::gpu::d3d11 {
    class ResourceViewD3D11 final : public ResourceView {
    public:
        explicit ResourceViewD3D11(ViewType type, com_ptr<ID3D11View> view);
        virtual ~ResourceViewD3D11();

        ResourceViewD3D11(ResourceViewD3D11&&) = delete;
        ResourceViewD3D11& operator=(ResourceViewD3D11&&) = delete;

        ViewType type() const override { return _type; }
        com_ptr<ID3D11View> const& getView() const { return _view; }

    private:
        ViewType _type;
        com_ptr<ID3D11View> _view;
    };
} // namespace gm::gpu::d3d11
