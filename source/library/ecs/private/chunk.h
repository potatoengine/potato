// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

struct up::World::Chunk {
    static constexpr uint32 size = 64 * 1024;

    struct alignas(32) Header {
        int count = 0;
        Chunk* next = nullptr;
    };
    using Payload = char[size - sizeof(Header)];

    Header header;
    Payload data;

    EntityId* entity(up::uint32 index) noexcept {
        return reinterpret_cast<EntityId*>(data + index * sizeof(EntityId));
    }

    constexpr void* pointer(Layout layout, up::uint32 index) noexcept {
        return data + layout.offset + index * layout.width;
    }
};
