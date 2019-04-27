// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct up::World::Layout {
    ComponentId component = ComponentId::Unknown;
    uint32 offset = 0;
    uint32 width = 0;
    ComponentMeta const* meta = nullptr;
};

struct up::World::Archetype {
    vector<box<Chunk>> chunks;
    vector<Layout> layout;
    uint64 layoutHash = 0;
    uint32 count = 0;
    uint32 perChunk = 0;

    int32 indexOfLayout(ComponentId component) const noexcept {
        for (int32 i = 0; i != static_cast<int32>(layout.size()); ++i) {
            if (layout[i].meta->id == component) {
                return i;
            }
        }
        return -1;
    }
};

struct up::World::Location {
    uint32 chunk = 0;
    uint32 index = 0;
};

namespace up {
    constexpr auto location(World::Archetype const& archetype, uint32 entityIndex) noexcept -> World::Location {
        World::Location loc;
        loc.chunk = entityIndex / archetype.perChunk;
        loc.index = entityIndex % archetype.perChunk;
        return loc;
    }
}
