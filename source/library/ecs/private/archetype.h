// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct up::World::Archetype {
    struct Layout {
        ComponentMeta const* meta = nullptr;
        uint32 offset = 0;
    };

    vector<box<Chunk>> chunks;
    vector<Layout> layout;
    uint32 count = 0;
    uint32 perChunk = 0;
};
