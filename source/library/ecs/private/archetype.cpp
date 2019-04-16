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
    // FIXME: handle Archetypes that have multiple copies of the same component
    for (ComponentId comp : components) {
        if (find(_layout, comp, {}, [](Layout const& layout) noexcept -> ComponentId { return layout.component; }) == _layout.end()) {
            return false;
        }
    }
    return true;
}

bool up::Archetype::matchesExact(view<ComponentId> components) const noexcept {
    if (components.size() != _layout.size()) {
        return false;
    }

    return matches(components);
}

void up::Archetype::unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const noexcept {
    void* pointers[64];
    UP_ASSERT(components.size() <= std::size(pointers));    

    for (size_t i = 0; i < components.size(); ++i) {
        pointers[i] = unsafeComponentPointer(0, components[i]);
    }

    callback(static_cast<size_t>(_count), view<void*>(pointers).first(components.size()));
}

void* up::Archetype::unsafeComponentPointer(uint32 entityIndex, ComponentId component) const noexcept {
    auto layoutIter = find(_layout, component, {}, [](Layout const& layout) noexcept { return layout.component; });
    UP_ASSERT(layoutIter != _layout.end());
    ComponentInfo info(component);
    return _components + layoutIter->offset + entityIndex * info.size;
}

auto up::Archetype::allocateEntity() noexcept -> uint32 {
    uint32 id = _count++;
    UP_ASSERT(_count < _perChunk);
    return id;
}

auto up::Archetype::unsafeAllocate(view<ComponentId> componentIds, view<void const*> componentData) noexcept -> uint32 {
    UP_ASSERT(componentData.size() == _layout.size());
    UP_ASSERT(componentIds.size() == componentData.size());

    uint32 entityIndex = allocateEntity();

    for (uint32 index = 0; index != componentIds.size(); ++index) {
        ComponentInfo info(componentIds[index]);
        std::memcpy(unsafeComponentPointer(entityIndex, componentIds[index]), componentData[index], info.size);
    }

    return entityIndex;
}
