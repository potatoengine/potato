// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/resource.h"

namespace gm::gpu::d3d11 {
    class ResourceD3D11 final : public GpuResource {
    public:
        explicit ResourceD3D11(com_ptr<ID3D11Resource> resource);
        virtual ~ResourceD3D11();

        ResourceD3D11(ResourceD3D11&&) = delete;
        ResourceD3D11& operator=(ResourceD3D11&&) = delete;

        com_ptr<ID3D11Resource> const& get() const { return _resource; }

    private:
        com_ptr<ID3D11Resource> _resource;
    };
} // namespace gm::gpu::d3d11
