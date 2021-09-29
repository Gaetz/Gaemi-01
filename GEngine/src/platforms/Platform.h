//
// Created by gaetz on 21/09/2021.
//

#ifndef VK_PLATFORM_H
#define VK_PLATFORM_H

#include <string>
using std::string;

#include <array>
using std::array;

#include "../Defines.h"

namespace engine::platforms {

class Platform {
public:
    virtual ~Platform() = default;

    virtual bool init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) = 0;
    GAPI virtual void update(u32 dt) = 0;
    virtual void shutdown() = 0;
    virtual bool pumpMessages() = 0;
    virtual void* getScreenSurface() = 0;

    // TODO Process input in a portable manner

    virtual void* allocate(u64 size, bool aligned) = 0;
    virtual void free(void* block, bool aligned) = 0;
    virtual void* zeroMemory(void* block, u64 size) = 0;
    virtual void* copyMemory(void* dest, const void* source, u64 size) = 0;
    virtual void* setMemory(void* dest, i32 value, u64 size) = 0;

    virtual void consoleWrite(const string& message, u8 color) = 0;
    virtual void consoleWriteError(const string& message, u8 color) = 0;

    virtual u64 getAbsoluteTimeMs() = 0;
    virtual f64 getAbsoluteTimeSeconds() = 0;
    virtual void sleep(u32 ms) = 0;
    virtual array<char, 19> getDate() = 0;
};

}

#endif