// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/resource.h"

namespace gm {
    class D3d12Resource final : public GpuResource {
    public:
        explicit D3d12Resource(com_ptr<ID3D12Resource> resource);
        virtual ~D3d12Resource();

        D3d12Resource(D3d12Resource&&) = delete;
        D3d12Resource& operator=(D3d12Resource&&) = delete;

        com_ptr<ID3D12Resource> const& get() const { return _resource; }

    private:
        com_ptr<ID3D12Resource> _resource;
    };
} // namespace gm
