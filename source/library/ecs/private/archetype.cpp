// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"
#include "potato/ecs/domain.h"
#include <algorithm>

static constexpr size_t align(size_t offset, size_t alignment) noexcept {
    size_t alignmentMinusOne = alignment - 1;
    return (offset + alignmentMinusOne) & ~alignmentMinusOne;
}

up::Archetype::Archetype(EntityDomain& domain, view<ComponentId> comps) noexcept : _domain(domain), _layout(comps.size()) {
    size_t size = 0;
    for (size_t i = 0; i != comps.size(); ++i) {
        ComponentInfo info(comps[i]);
        _layout[i].component = comps[i];
        size = align(size, info.alignment);
        size += info.size;
    }
    UP_ASSERT(size <= sizeof(EntityChunk::data));

    if (size != 0) {
        // assign pointer offers by alignment
        std::sort(_layout.begin(), _layout.end(), [](const auto& l, const auto& r) noexcept { return ComponentInfo(l.component).alignment < ComponentInfo(l.component).alignment; });

        _perChunk = static_cast<uint32>(sizeof(EntityChunk::Payload) / size);

        size_t offset = 0;
        for (size_t i = 0; i != comps.size(); ++i) {
            ComponentInfo info(comps[i]);

            // align as required (requires alignment to be a power of 2)
            UP_ASSERT((info.alignment & (info.alignment - 1)) == 0);
            offset = align(offset, info.alignment);

            _layout[i].offset = static_cast<uint32>(offset);

            offset += info.size * _perChunk;
        }

        UP_ASSERT(offset <= sizeof(EntityChunk::data));
    }

    // layout must be stored by ComponentId
    std::sort(_layout.begin(), _layout.end(), [](const auto& l, const auto& r) noexcept { return l.component < r.component; });
}

up::Archetype::~Archetype() = default;

bool up::Archetype::matches(view<ComponentId> components) const noexcept {
    // FIXME: handle Archetypes that have multiple copies of the same component
    for (ComponentId comp : components) {
        if (find(_layout, comp, {}, [](Layout const& layout) noexcept->ComponentId { return layout.component; }) == _layout.end()) {
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

void up::Archetype::unsafeSelect(view<ComponentId> components, delegate_ref<SelectSignature> callback) const {
    void* pointers[64];
    UP_ASSERT(components.size() <= std::size(pointers));

    for (size_t i = 0; i < components.size(); ++i) {
        pointers[i] = unsafeComponentPointer(0, components[i]);
    }

    callback(static_cast<size_t>(_count), view<void*>(pointers).first(components.size()));
}

void* up::Archetype::unsafeComponentPointer(up::uint32 entityIndex, ComponentId component) const noexcept {
    auto layoutIter = find(_layout, component, {}, [](Layout const& layout) noexcept { return layout.component; });
    UP_ASSERT(layoutIter != _layout.end());
    ComponentInfo info(component);

    auto chunkIndex = entityIndex / _perChunk;
    auto subIndex = entityIndex % _perChunk;

    return _chunks[chunkIndex]->data + layoutIter->offset + entityIndex * info.size;
}

auto up::Archetype::unsafeRemoveEntity(up::uint32 entityIndex) noexcept -> EntityId {
    auto chunkIndex = entityIndex / _perChunk;
    auto subIndex = entityIndex % _perChunk;

    UP_ASSERT(chunkIndex < _chunks.size());
    UP_ASSERT(subIndex < _perChunk);

    auto lastChunkIndex = _chunks.size() - 1;
    auto lastSubIndex = _chunks[lastChunkIndex]->header.count - 1;
    EntityId lastEntity = _entities.back();

    if (lastChunkIndex != chunkIndex || lastSubIndex != subIndex) {
        _entities[entityIndex] = _entities.back();

        for (auto const& layout : _layout) {
            ComponentInfo info(layout.component);
            void* pointer = _chunks[chunkIndex]->data + layout.offset + subIndex * info.size;
            void const* lastPointer = _chunks[lastChunkIndex]->data + layout.offset + lastSubIndex * info.size;
            std::memcpy(pointer, lastPointer, info.size);
        }
    }

    if (--_chunks[lastChunkIndex]->header.count == 0) {
        _domain.returnChunk(std::move(_chunks[lastChunkIndex]));
        _chunks.pop_back();
    }
    _entities.pop_back();

    return lastEntity;
}

auto up::Archetype::unsafeAllocate(EntityId entity, view<ComponentId> componentIds, view<void const*> componentData) noexcept -> up::uint32 {
    UP_ASSERT(componentData.size() == _layout.size());
    UP_ASSERT(componentIds.size() == componentData.size());

    uint32 entityIndex = _count++;

    auto chunkIndex = entityIndex / _perChunk;
    auto subIndex = entityIndex % _perChunk;

    UP_ASSERT(chunkIndex <= _chunks.size());

    _entities.push_back(entity);
    if (chunkIndex == _chunks.size()) {
        _chunks.push_back(_domain.allocateChunk());
    }

    ++_chunks[chunkIndex]->header.count;

    for (uint32 index = 0; index != componentIds.size(); ++index) {
        ComponentId componentId = componentIds[index];
        ComponentInfo info(componentId);

        auto layoutIter = find(_layout, componentId, {}, [](Layout const& layout) noexcept { return layout.component; });
        UP_ASSERT(layoutIter != _layout.end());

        void* rawPointer = _chunks[chunkIndex]->data + layoutIter->offset + subIndex * info.size;
        UP_ASSERT(rawPointer >= _chunks[chunkIndex]->data);
        UP_ASSERT(rawPointer <= _chunks[chunkIndex]->data + sizeof(EntityChunk::Payload) - info.size);

        std::memcpy(rawPointer, componentData[index], info.size);
    }

    return entityIndex;
}
