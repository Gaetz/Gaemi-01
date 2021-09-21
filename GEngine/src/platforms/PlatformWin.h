//
// Created by gaetz on 21/09/2021.
//

#ifndef VK_PLATFORM_WIN_H
#define VK_PLATFORM_WIN_H

#include "Platform.h"
#include "Window.h"

namespace engine::platforms {

class PlatformWin : public Platform {
public:
    PlatformWin() = default;
    virtual ~PlatformWin();

    virtual b8 init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) override;
    virtual void update(u64 dt) override;
    virtual void shutdown() override;
    virtual b8 pumpMessages() override;
    virtual void* getScreenSurface() override;

    virtual void* allocate(u64 size, b8 aligned) override;
    virtual void free(void* block, b8 aligned) override;
    virtual void* zeroMemory(void* block, u64 size) override;
    virtual void* copyMemory(void* dest, const void* source, u64 size) override;
    virtual void* setMemory(void* dest, i32 value, u64 size) override;

    virtual void consoleWrite(const string& message, u8 color) override;
    virtual void consoleWriteError(const string& message, u8 color) override;

    virtual f64 getAbsoluteTime() override;
    virtual void sleep(u64 ms) override;

protected:
    Window window { "Gaemi-01" };
};

}

#endif