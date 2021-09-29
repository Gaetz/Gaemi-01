//
// Created by gaetz on 26/09/2021.
//

#include "Memory.h"
#include "Log.h"
#include "Engine.h"

using engine::Memory;

void Memory::init(Platform* platformP) {
    platform = platformP;
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
    MemoryQuantity totalMem = computeUnitAndAmount(totalAllocated);
    LOG(LogLevel::Info) << "Total memory allocations: " << totalMem.amount << " " << totalMem.unit.data();
    for(i32 i = 0; i < MEMORY_TAG_MAX; ++i) {
        if(taggedAllocations[i] > 0) {
            MemoryQuantity taggedMem = computeUnitAndAmount(taggedAllocations[i]);
            LOG(LogLevel::Trace) << memoryTagToString(i) << ": " << taggedMem.amount << " " << taggedMem.unit.data();
        }
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

void Memory::addAllocated(u64 size, engine::MemoryTag tag) {
    totalAllocated += size;
    taggedAllocations[static_cast<i32>(tag)] += size;
}

engine::MemoryQuantity engine::Memory::computeUnitAndAmount(u64 size) {
    const u64 gb = 1024 * 1024 * 1024;
    const u64 mb = 1024 * 1024;
    const u64 kb = 1024;

    MemoryQuantity mem;
    if (size >= gb) {
        mem.unit[0] = 'G';
        mem.amount = (float)size / (float)gb;
    } else if (size >= mb) {
        mem.unit[0] = 'M';
        mem.amount = (float)size / (float)mb;
    } else if (size >= kb) {
        mem.unit[0] = 'k';
        mem.amount = (float)size / (float)kb;
    } else {
        mem.unit[0] = 'B';
        mem.unit[1] = ' ';
        mem.amount = (float)size;
    }
    return mem;
}
