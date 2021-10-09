//
// Created by gaetz on 06/10/2021.
//

#ifndef MEMORY_H
#define MEMORY_H

#include "MemoryTag.h"


namespace engine::mem {

    class Memory {
    public:
        GAPI virtual void *allocate(u64 size, MemoryTag tag) = 0;

        GAPI virtual void free(void *block, u64 size, MemoryTag tag) = 0;

        GAPI virtual void *zero(void *block, u64 size) = 0;

        GAPI virtual void *copy(void *dest, const void *source, u64 size) = 0;

        GAPI virtual void *set(void *dest, i32 value, u64 size) = 0;

        GAPI virtual void logMemoryUsage() = 0;
    };

    class NullMemory : public Memory {
    public:
        GAPI void *allocate(u64 size, MemoryTag tag) override {
            placeholderMessage();
            return nullptr;
        };

        GAPI void free(void *block, u64 size, MemoryTag tag) override {
            placeholderMessage();
        };

        GAPI void *zero(void *block, u64 size) override {
            placeholderMessage();
            return nullptr;
        };

        GAPI void *copy(void *dest, const void *source, u64 size) override {
            placeholderMessage();
            return nullptr;
        };

        GAPI void *set(void *dest, i32 value, u64 size) override {
            placeholderMessage();
            return nullptr;
        };

        GAPI void logMemoryUsage() override {
            placeholderMessage();
        };

    private:
        void placeholderMessage() {
            LOG(Warning) << "Usage of placeholder memory service.";
        }
    };
}



#endif //MEMORY_H
