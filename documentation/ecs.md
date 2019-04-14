Entity Component System Architecture
====================================

Concepts
--------

The Potato ECS is based on the use of Archetypes. Key definitions for this design:

- **Entity**: an abstract representation of a "game object" that exists
 in the simulation
- **Component**: a discrete chunk of plain data used to describe some aspect of an Entity
- **Tag**: a Component with no data
- **System**: a function that operates on Components to mutate data and progress the simulation
- **Archetype**: a classification of Entity with a specific set of Component types; all Entities with the same types of Components will share the same Archetype
- **Query**: a selector to find all Archetypes which include a particular set of Components
- **World**: a container for Components, Entities, and Archetypes; Entities in different Worlds cannot interact
- **Layout**: the specific in-memory arrangement of Components for an Archetype; several Archetypes may share a Layout if they differ only by Tags
- **Chunk**: the storage arena for Components associated with a particular Archetype

Abstract Description
--------------------

Potato ECS follows the "pure" definition of Entity Component System architecture. In Potato, a **Component** is just a plain holder of data, typically a `struct` with some fields. Some Components may not contain any data at all and are called **Tag** Components.

Components contain no logic at all. For operating on Component data, a **System** is used. Systems are functions or classes which operation on Components, grouped together by logical **Entity**. An Entity is just a way of associating Components together.

A System will typically use a **Query** asking to find Entities with some specific set of Components. For example, a rendering System might want to find all Entities with a `Transform` Component and a `Mesh` Component, independent of whatever other Components those Entities might have. A Query is used to express this requirement and searches for compatible Entities.

A Query does not actually search for Entities. Instead, it searches for **Archetypes**. An Archetype is a grouping of Entities with identical sets of Components. For example, Entities which represent static props in a game simulation might have the Components `{Transform,Mesh}`; a set of Entities for dynamic props might have `{Transform,Physics,Mesh}`; and a set of Entities for static invisible trigger volumes might have `{Transform,Collider,Script}` Components. The aforementioned rendering System would use a Query looking for `{Transform,Mesh}`, which would then find the first two archetypes but not the third (it has no `Mesh` Component). Because there will typically be many Entities with the same Components, there will be far fewer Archetypes than Entities, and searching for Archetypes will be much faster than searching all Entities.

The real benefit of this approach is enabled by the concept of **Chunks**. All the Components belonging to Entities in a single Archetype are allocated together in a series of **Chunks**. The placement of Components without a **Chunk** is determined by the Archetype's **Layout**, which can be thought of similarly to Input Assemblies used in graphics shaders. All Components of a given type are allocated contiguously in each Chunk, with the specific offsets/strides described by the Layout.

Because Components are allocated together in their associated Archetype, actually operating on Components can be done very efficiently. After a Query finds the set of compatible Archetypes, it can use the Layout and Chunks for each Archetype to pass pointers to the System for performing work. Because multiple Chunks might be present - not to mention multiple Archetypes with different Layouts - a System might be invoked multiple times with different pointers. We can ensure that System is invoked once for each Chunk that contains Components belonging to Entities compatible with the System's requested Query.

Continuing the prior example, the render System would be invoked (at least) twice: once with pointers for the `Transform` and `Mesh` Components stored in the first Archetype's Chunk(s), and then again with the pointers for the Components stored in the second Archetype's Chunk(s).

When operating on large numbers of similar Entities, this approach will result in very efficient use of the CPU and memory caches. When operating on relatively "unique" Entities, efficiency will be lower, but should be no worse than more traditional component-based engine designs.

Archetypes, Layouts, and Chunks
-------------------------------

For traditional ECS, the architecture calls for all Components to be allocated contiguously. This allows operating on a specific Component to be highly cache-efficient. However, difficulties arise when needing to operate on multiple Components simultaneously; the engine needs to iterate multiple contiguous Component allocations while associating those Components together. Traditional approaches have involved hash tables, sorted containers, and other algorithmic solutions.

The solution used by Potato instead relies on allocating associated Components separately. A single Component type, such as `Transform`, might then exist in multiple contiguous arrays, depending on their Entity's associated Components. This ensures a direct mapping between associated Components via index into their respective arrays.

The concept of an Archetype is used to determine which Components are asociated and require their arrays to be colocated. The set of Components belonging to an Entity is used to identify the Entity's Archetype. The non-Tag Components are then used to define a Layout for that Archetype. The Layout defines the offsets and strides for arrays containing associated non-Tag Components.

The actual Components are stored in Chunks. A Chunk is just a hunk of memory (e.g. a 64kb block) owned by a particular Layout; it is sliced into arrays of Components as directed by the Layout.

For example, let's assume some Entities with the Components `{Static,Mesh,Transform}`. That set of Components defines an Archetype, which for convenience we'll call the *StaticMeshTransformArchetype*. Let's further assume that `Static` is a Tag and holds no data. Since the Layout only includes the non-Tag Components `{Mesh,Transform}`, which we'll simply call the *MeshTransformLayout*. The Chunks allocated for this Layout would look something like this in memory:

    |-----------------------------------------|
    | Mesh      | Mesh      | ... | Mesh      |
    | Transform | Transform | ... | Transform |
    |-----------------------------------------|

If we inspected the *MeshTransformLayout*, we'd see that it says its Chunks contain the `Mesh` Component at offset 0 and the `Transform` Component at offset `sizeof(Mesh)`, and notes how many of these can be allocated in each Chunk (something like `sizeof(Chunk)/(sizeof(Mesh)+sizeof(Transform))`).

Let us assume then that the game wants to find all static meshes in the game and bake them into a static vertex batch. It would execute a query for `{Static,Mesh,Tranform}`. In this case that's exactly identical to our *StaticMeshTransformArchetype*, so it will find that archetype.

Alternatively, let's ignore static batching, and assume a system just wants to find all meshes in the game and render them. It would issue a Query for `{Mesh,Tranform}`, which also finds out our *StaticMeshTransformArchetype* because it has both of the requested Components.

Either way, once the Archetype is found, the Query then has access to the *MeshTransformLayout* and all the Chunks that have been allocated for it thus far. The Query produces a pair of `Mesh*` and `Transform*` pointers for each Chunk.

Let's assume that there's another Archetype, the *ProjectilePhysicsMeshTranformArchetype*, whose Layout describes Chunks that look like:

    |--------------------------------------------|
    | Projectile | Projectile | ... | Projectile |
    | Physics    | Physics    | ... | Physics    |
    | Mesh       | Mesh       | ... | Mesh       |
    | Transform  | Transform  | ... | Transform  |
    |--------------------------------------------|

Note that the offsets of `Mesh` and `Transform` are different, and the maximum number of Components per Chunk will be smaller.

Our Query for `{Mesh,Tranform}` would also find this Archetype, and pairs of `Mesh*` and `Transform*` pointers would be found for each Chunk belonging to *ProjectilePhysicsMeshTranformArchetype*.

The System's logic is then invoked for each of these pairs of pointers, along with a count; different Chunks can fit a different maximum number of Components (based on Layout), and some Chunks may not have full residency (e.g., if only a few Entities for a given Archetype exist, there may not be a full Chunk's worth of Components).

Because the System is just given pointers and a count, it doesn't need to know anything at all about Archetypes or Layouts or Chunks. It just iterates over the Components at the provided spans of memory.
