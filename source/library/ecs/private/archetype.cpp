// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"

up::Archetype::Archetype(view<ComponentId> comps) noexcept : _layout(comps.size()) {
    size_t size = 0;
    for (size_t i = 0; i != comps.size(); ++i) {
        _layout[i].component = comps[i];
        size += ComponentInfo{comps[i]}.size;
    }

    if (size == 0) {
        _perChunk = 0;
        _components = nullptr;
        return;
    }

    _perChunk = static_cast<uint32>(_chunkSize / size);

    size_t offset = 0;
    for (size_t i = 0; i != comps.size(); ++i) {
        ComponentInfo info(comps[i]);

        // align as required (requires alignment to be a power of 2)
        UP_ASSERT((info.alignment & (info.alignment - 1)) == 0);
        size_t alignmentMinusOne = info.alignment - 1;
        offset = (offset + alignmentMinusOne) & ~alignmentMinusOne;

        _layout[i].offset = static_cast<uint32>(offset);

        offset += info.size * _perChunk;
    }

    UP_ASSERT(offset <= _chunkSize);

    _components = new char[_chunkSize];
}

up::Archetype::~Archetype() {
    delete[] _components;
}

bool up::Archetype::matches(view<ComponentId> components) const noexcept {
    for (ComponentId comp : components) {
        if (find(_layout, comp, {}, [](Layout const& layout) noexcept -> ComponentId { return layout.component; }) == _layout.end()) {
            return false;
        }
    }
    return true;
}

void up::Archetype::unsafeSelect(Query const& query, delegate_ref<SelectSignature> callback) const noexcept {
    void* pointers[64];

    auto queryComponents = query.components();

    for (size_t i = 0; i < queryComponents.size(); ++i) {
        auto layoutIter = find(_layout, queryComponents[i], {}, [](Layout const& layout) noexcept { return layout.component; });
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
