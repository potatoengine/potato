// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct up::World::Archetype {
    struct Layout {
        ComponentId component = ComponentId::Unknown;
        uint32 offset = 0;
    };

    vector<box<Chunk>> _chunks;
    vector<Layout> _layout;
    uint32 _count = 0;
    uint32 _perChunk = 0;
};
