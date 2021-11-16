//
// Created by gaetz on 26/09/2021.
//

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <array>
using std::array;

#include "../Defines.h"
#include "MemoryTag.h"
#include "Memory.h"


namespace engine::platforms {
    class Platform;
}

// TODO Replace with C++ custom allocator and heap management system

namespace engine::mem {

    struct MemoryQuantity {
        float amount { 1.0f };
        array<char, 3> unit {' ', 'B', '\0'};
    };

    class MemoryManager : public Memory {
    public:

        MemoryManager() = default;
        ~MemoryManager() = default;
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager(MemoryManager&&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;
        MemoryManager& operator=(MemoryManager&&) = delete;

        void init(platforms::Platform* platformP);
        void close();

        void* allocate(u64 size, MemoryTag tag) override;
        void free(void* block, u64 size, MemoryTag tag) override;
        void* zero(void* block, u64 size) override;
        void* copy(void* dest, const void* source, u64 size) override;
        void* set(void* dest, i32 value, u64 size) override;
        void logMemoryUsage() override;

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
        std::array<u64, MEMORY_TAG_MAX> taggedAllocations {};

        std::string memoryTagToString(i32 i);
        MemoryQuantity computeUnitAndAmount(u64 size);
    };
}



#endif //MEMORY_MANAGER_H
