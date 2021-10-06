//
// Created by gaetz on 26/09/2021.
//

#ifndef MEMORY_H
#define MEMORY_H

#include <array>
using std::array;

#include "platforms/Platform.h"
#include "Defines.h"

namespace engine {

    enum class MemoryTag {
        Unknown,
        Array,
        DArray,
        Dict,
        RingQueue,
        Bst,
        String,
        Application,
        Job,
        Texture,
        MaterialInstance,
        Renderer,
        Game,
        Transform,
        Entity,
        EntityNode,
        Scene,

        MaxMemoryTag
    };

    constexpr u8 MEMORY_TAG_MAX = static_cast<u8>(MemoryTag::MaxMemoryTag);

    struct MemoryQuantity {
        float amount { 1.0f };
        array<char, 3> unit {' ', 'B', '\0'};
    };

    class MemoryManager {
    public:

        MemoryManager() = default;
        ~MemoryManager() = default;
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager(MemoryManager&&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;
        MemoryManager& operator=(MemoryManager&&) = delete;

        void init(platforms::Platform* platformP);
        void close();

        GAPI void* allocate(u64 size, MemoryTag tag);
        GAPI void free(void* block, u64 size, MemoryTag tag);
        GAPI void* zero(void* block, u64 size);
        GAPI void* copy(void* dest, const void* source, u64 size);
        GAPI void* set(void* dest, i32 value, u64 size);
        GAPI void logMemoryUsage();

        /**
         * Used to count memory that was not allocated through allocate function.
         * Example: count game instance memory.
         * @param size Size on the allocated memory
         * @param tag Tag of the allocated memory
         */
        void addAllocated(u64 size, MemoryTag tag);

    private:
        platforms::Platform* platform { nullptr };
        u64 totalAllocated { 0 };
        array<u64, MEMORY_TAG_MAX> taggedAllocations {};

        std::string memoryTagToString(i32 i);
        MemoryQuantity computeUnitAndAmount(u64 size);
    };

}


#endif //MEMORY_H
