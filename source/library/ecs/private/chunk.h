// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct up::World::Chunk {
    static constexpr uint32 size = 64 * 1024;

    struct alignas(32) Header {
        int count = 0;
    };
    using Payload = char[size - sizeof(Header)];

    Header header;
    Payload data;
};
