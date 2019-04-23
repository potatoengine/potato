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
};

struct up::World::Location {
    uint32 chunk = 0;
    uint32 index = 0;
};
