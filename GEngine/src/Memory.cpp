//
// Created by gaetz on 26/09/2021.
//

#include "Memory.h"
#include "Log.h"
#include "Engine.h"

using engine::Memory;

void Memory::init() {
    platform = Engine::getState().platform;
}

void Memory::close() {

}

void *Memory::allocate(u64 size, MemoryTag tag) {
    // Warn in case of unknown tag
    if (tag == MemoryTag::Unknown) {
        LOG(LogLevel::Warning) << "Allocate used with MemoryTag::Unknown. Please reclass this allocation.";
    }
    // Count memory
    totalAllocated += size;
    taggedAllocations[static_cast<i32>(tag)] += size;
    // Get fresh zero-ed memory
    void* block = platform->allocate(size, false);
    platform->zeroMemory(block, size);
    return block;
}

void Memory::free(void *block, u64 size, MemoryTag tag) {
    // Warn in case of unknown tag
    if (tag == MemoryTag::Unknown) {
        LOG(LogLevel::Warning) << "Allocate used with MemoryTag::Unknown. Please reclass this allocation.";
    }
    // Count memory
    totalAllocated -= size;
    taggedAllocations[static_cast<i32>(tag)] -= size;
    // Free memory
    Engine::getState().platform->free(block, false);
}

void *Memory::zero(void *block, u64 size) {
    return platform->zeroMemory(block, size);
}

void *Memory::copy(void *dest, const void *source, u64 size) {
    return platform->copyMemory(dest, source, size);
}

void *Memory::set(void *dest, i32 value, u64 size) {
    return platform->setMemory(dest, value, size);
}

void Memory::logMemoryUsage() {
    const u64 gb = 1024 * 1024 * 1024;
    const u64 mb = 1024 * 1024;
    const u64 kb = 1024;

    for(i32 i = 0; i < MEMORY_TAG_MAX; ++i) {
        array<char, 3> unit {' ', 'B', '\0'};
        float amount = 1.0f;
        if (taggedAllocations[i] >= gb) {
            unit[0] = 'G';
            amount = (float)taggedAllocations[i] / (float)gb;
        } else if (taggedAllocations[i] >= mb) {
            unit[0] = 'M';
            amount = (float)taggedAllocations[i] / (float)mb;
        } else if (taggedAllocations[i] >= kb) {
            unit[0] = 'k';
            amount = (float)taggedAllocations[i] / (float)kb;
        } else {
            unit[0] = 'B';
            unit[1] = ' ';
            amount = (float)taggedAllocations[i];
        }
        LOG(LogLevel::Info) << memoryTagToString(i) << ": " << taggedAllocations[i] << unit.data();
    }
}

std::string Memory::memoryTagToString(i32 i) {
    switch(i) {
        case 0: return "Unknown";
        case 1: return "Array";
        case 2: return "DArray";
        case 3: return "Dict";
        case 4: return "RingQueue";
        case 5: return "Bst";
        case 6: return "String";
        case 7: return "Application";
        case 8: return "Job";
        case 9: return "Texture";
        case 10: return "MaterialInstance";
        case 11: return "Renderer";
        case 12: return "Game";
        case 13: return "Transform";
        case 14: return "Entity";
        case 15: return "EntityNode";
        case 16: return "Scene";
        default: return "ERROR Memory tag not set";
    }
}
