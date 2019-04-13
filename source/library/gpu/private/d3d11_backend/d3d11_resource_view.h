// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/foundation/box.h"
#include "potato/gpu/resource_view.h"

namespace up::gpu::d3d11 {
    class ResourceViewD3D11 final : public GpuResourceView {
    public:
        explicit ResourceViewD3D11(GpuViewType type, com_ptr<ID3D11View> view);
        virtual ~ResourceViewD3D11();

        ResourceViewD3D11(ResourceViewD3D11&&) = delete;
        ResourceViewD3D11& operator=(ResourceViewD3D11&&) = delete;

        GpuViewType type() const override { return _type; }
        com_ptr<ID3D11View> const& getView() const { return _view; }

    private:
        GpuViewType _type;
        com_ptr<ID3D11View> _view;
    };
} // namespace up::gpu::d3d11
