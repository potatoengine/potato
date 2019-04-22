// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/ecs/archetype.h"
#include "potato/ecs/domain.h"
#include <algorithm>

static constexpr size_t align(size_t offset, size_t alignment) noexcept {
    size_t alignmentMinusOne = alignment - 1;
    return (offset + alignmentMinusOne) & ~alignmentMinusOne;
}

up::Archetype::Archetype(view<ComponentId> comps) noexcept : _layout(comps.size()) {
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
