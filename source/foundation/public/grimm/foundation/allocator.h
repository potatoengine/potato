// Copyright (C) 2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "assertion.h"
#include "memory.h"

namespace gm {

    /// <summary> The standard default allocator. </summary>
    struct default_allocator {
        [[nodiscard]] void* allocate(size_t size, size_t align = alignof(double)) {
            GM_ASSERT(align <= alignof(double), "Default allocator only supports alignment of up to that of a double");
            return GM_DEFAULT_ALLOC(size, align);
        }

        void deallocate(void* ptr, size_t size, size_t align = alignof(double)) {
            GM_DEFAULT_FREE(ptr, size, align);
        }
    };

    /// <summary> The standard string allocator. </summary>
    struct string_allocator {
        [[nodiscard]] void* allocate(size_t size, size_t align = 1) {
            GM_ASSERT(align == 1, "String allocator does not support alignment greater than 1");
            return GM_STRING_ALLOC(size, 1);
        }

        void deallocate(void* ptr, size_t size, size_t align = 1) {
            GM_ASSERT(align == 1, "String allocator does not support alignment greater than 1");
            GM_STRING_FREE(ptr, size, 1);
        }
    };

    template <typename T, typename Allocator = default_allocator>
    struct typed_allocator : Allocator {
        [[nodiscard]] T* allocate(size_t count) {
            return static_cast<T*>(Allocator::allocate(count * sizeof(T), alignof(T)));
        }

        void deallocate(void* ptr, size_t count) {
            Allocator::deallocate(ptr, count * sizeof(T), alignof(T));
        }
    };

    /// <summary> Allocates a contiguous block of memory sized and aligned for one or more copies of specific object. </summary>
    /// <typeparam name="T"> Type of the object to allocate. </typeparam>
    /// <typeparam name="Allocator"> The type of allocator to use. </typeparam>
    /// <param name="allocator"> The allocator. </param>
    /// <param name="count"> Number of contiguous copies of the object to allocate. </param>
    /// <returns> null if it fails, else a pointer to the objects. </returns>
    template <typename T, typename Allocator>
    [[nodiscard]] T* allocate(Allocator& allocator, size_t count) {
        return static_cast<T*>(allocator.allocate(sizeof(T) * count, alignof(T)));
    }

    /// <summary> Deallocates a contiguous block of memory sized and aligned for one or more copies of specific object. </summary>
    /// <typeparam name="T"> Type of the object to deallocate. </typeparam>
    /// <typeparam name="Allocator"> The type of allocator to use. </typeparam>
    /// <param name="allocator"> The allocator which allocated the block of memory. </param>
    /// <param name="ptr"> The memory block to deallocate. </param>
    /// <param name="count"> Number of objects the block was allocated for. </param>
    template <typename T, typename Allocator>
    void deallocate(Allocator& allocator, T* ptr, size_t count) {
        allocator.deallocate(ptr, sizeof(T) * count, alignof(T));
    }

} // namespace gm
