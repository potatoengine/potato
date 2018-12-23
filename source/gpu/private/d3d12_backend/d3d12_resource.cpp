// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_resource.h"
#    include "com_ptr.h"
#    include "direct3d.h"
#    include "grimm/foundation/box.h"
#    include "grimm/foundation/logging.h"
#    include "grimm/foundation/out_ptr.h"
#    include <utility>

gm::D3d12Resource::D3d12Resource(com_ptr<ID3D12Resource> resource) : _resource(std::move(resource)) {}

gm::D3d12Resource::~D3d12Resource() = default;

#endif
