//
// Created by gaetz on 21/09/2021.
//

#ifndef VK_PLATFORM_WIN_H
#define VK_PLATFORM_WIN_H

#include <windows.h>
#include <windowsx.h>

#include "Platform.h"
#include "Window.h"

namespace engine::platforms {

class PlatformWin : public Platform {
public:
    PlatformWin() = default;
    ~PlatformWin() override;

    b8 init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) override;
    void update(u64 dt) override;
    void shutdown() override;
    b8 pumpMessages() override;
    void* getScreenSurface() override;

    void* allocate(u64 size, b8 aligned) override;
    void free(void* block, b8 aligned) override;
    void* zeroMemory(void* block, u64 size) override;
    void* copyMemory(void* dest, const void* source, u64 size) override;
    void* setMemory(void* dest, i32 value, u64 size) override;

    void consoleWrite(const string& message, u8 color) override;
    void consoleWriteError(const string& message, u8 color) override;

    u32 getAbsoluteTimeMs() override;
    f64 getAbsoluteTimeSeconds() override;
    void sleep(u64 ms) override;
    array<char, 19> getDate() override;

protected:
    Window window { "Gaemi-01" };
};

}

#endif