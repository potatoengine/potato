// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

/// Chunks are the storage mechanism of Entities and their Components. A Chunk
/// is allocated to an Archetype and will store a list of Components according
/// to the Archetype's specified layout.
struct up::World::Chunk {
    /// Total size of a Chunk in bytes.
    static constexpr uint32 size = 64 * 1024;

    /// The fixed header at the beginning of every Chunk
    struct alignas(32) Header {
        unsigned int count = 0;
        box<Chunk> next;
    };
    using Payload = char[size - sizeof(Header)];

    Header header;
    Payload data;

    /// Calculates the location of an EntityId for a given index into the Chunk
    EntityId* entity(up::uint32 index) noexcept {
        return reinterpret_cast<EntityId*>(data + index * sizeof(EntityId));
    }

    /// Calculates the location of a Component for a given index into the Chunk
    constexpr void* pointer(Layout layout, up::uint32 index) noexcept {
        return data + layout.offset + index * layout.width;
    }
};
