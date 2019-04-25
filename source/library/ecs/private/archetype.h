// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct up::World::Layout {
    ComponentMeta const* meta = nullptr;
    uint32 offset = 0;
    uint32 width = 0;
};

struct up::World::Archetype {
    vector<box<Chunk>> chunks;
    vector<Layout> layout;
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
