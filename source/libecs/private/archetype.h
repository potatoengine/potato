// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

/// Describes the information about how components are laid out in an Archetype
struct up::World::Layout {
    ComponentId component = ComponentId::Unknown;
    ComponentMeta const* meta = nullptr;
    uint16 offset = 0;
    uint16 width = 0;
};

/// Describes the composition and memory layout of an Archetype
struct up::World::Archetype {
    vector<box<Chunk>> chunks;
    vector<Layout> layout;
    uint64 layoutHash = 0;
    uint32 count = 0;
    uint32 perChunk = 0;

    /// Returns index of the Layout for a Component.
    ///
    /// @return index of component or -1 if no match.
    ///
    int32 indexOfLayout(ComponentId component) const noexcept {
        for (int32 i = 0; i != static_cast<int32>(layout.size()); ++i) {
            if (layout[i].component == component) {
                return i;
            }
        }
        return -1;
    }
};

/// Identififies the location of an Entity within the Archetype/Chunk storage
struct up::World::Location {
    Archetype* archetype = nullptr;
    Chunk* chunk = nullptr;
    uint16 archetypeIndex = 0;
    uint16 chunkIndex = 0;
    uint32 subIndex = 0;
    uint32 entityIndex = 0;
};
