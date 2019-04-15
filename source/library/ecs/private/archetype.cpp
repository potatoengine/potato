// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"

up::Archetype::Archetype(view<ComponentInfo> comps) noexcept : _layout(comps.size()) {
    uint32 size = 0;
    for (size_t i = 0; i != comps.size(); ++i) {
        size += comps[i].size;
    }

    _perChunk = _chunkSize / size;

    uint32 offset = 0;
    for (size_t i = 0; i != comps.size(); ++i) {
        // align as required (requires alignment to be a power of 2)
        UP_ASSERT((comps[i].alignment & (comps[i].alignment - 1)) == 0);
        uint32 alignmentMinusOne = comps[i].alignment - 1;
        offset = (offset + alignmentMinusOne) & ~alignmentMinusOne;

        _layout[i].componentInfo = comps[i];
        _layout[i].offset = offset;

        offset += comps[i].size * _perChunk;
    }

    UP_ASSERT(offset <= _chunkSize);

    _components = new char[_chunkSize];
}

up::Archetype::~Archetype() {
    delete[] _components;
}

bool up::Archetype::matches(Query const& query) const noexcept {
    for (ComponentId comp : query.components()) {
        if (find(_layout, comp, {}, [](Layout const& layout) noexcept { return layout.componentInfo.id; }) == _layout.end()) {
            return false;
        }
    }
    return true;
}

void up::Archetype::unsafeSelect(Query const& query, delegate_ref<SelectSignature> callback) const noexcept {
    void* pointers[64];

    auto queryComponents = query.components();

    for (size_t i = 0; i < queryComponents.size(); ++i) {
        auto layoutIter = find(_layout, queryComponents[i], {}, [](Layout const& layout) noexcept { return layout.componentInfo.id; });
        UP_ASSERT(layoutIter != _layout.end());

        pointers[i] = _components + layoutIter->offset;
    }

    callback(static_cast<size_t>(_count), view<void*>(pointers).first(queryComponents.size()));
}

auto up::Archetype::allocateEntity() noexcept -> uint32 {
    uint32 id = _count++;
    UP_ASSERT(_count < _perChunk);
    return id;
}
