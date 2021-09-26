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

        MaxNumber
    };

    constexpr u8 MEMORY_TAG_MAX = static_cast<u8>(MemoryTag::MaxNumber);

    class Memory {
    public:

        static Memory& instance() {
            static Memory memoryInstance;
            return memoryInstance;
        }

        ~Memory() = default;

        void init();
        void close();

        GAPI void* allocate(u64 size, MemoryTag tag);
        GAPI void free(void* block, u64 size, MemoryTag tag);
        GAPI void* zero(void* block, u64 size);
        GAPI void* copy(void* dest, const void* source, u64 size);
        GAPI void* set(void* dest, i32 value, u64 size);
        GAPI void logMemoryUsage();

    private:
        platforms::Platform* platform { nullptr };
        u64 totalAllocated { 0 };
        array<u64, MEMORY_TAG_MAX> taggedAllocations {};

        Memory() = default;

        std::string memoryTagToString(i32 i);
    };

}


#endif //MEMORY_H
