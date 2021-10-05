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

    bool init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) override;
    void update(u32 dt) override;
    void close() override;
    bool pumpMessages() override;
    void* getScreenSurface() override;

    void* allocate(u64 size, bool aligned) override;
    void free(void* block, bool aligned) override;
    void* zeroMemory(void* block, u64 size) override;
    void* copyMemory(void* dest, const void* source, u64 size) override;
    void* setMemory(void* dest, i32 value, u64 size) override;

    void consoleWrite(const string& message, u8 color) override;
    void consoleWriteError(const string& message, u8 color) override;

    u64 getAbsoluteTimeMs() override;
    f64 getAbsoluteTimeSeconds() override;
    void sleep(u32 ms) override;
    array<char, 19> getDate() override;

    const u8* inputKeyboardGetState(i32* keys) override;
    i32 inputMouseGetButtonMask(i32 button) override;
    i32 inputControllerGetButton(void* controllerPtr, i32 button) override;
    i32 inputControllerGetAxis(void* controllerPtr, input::ControllerAxis axis) override;
    void* inputControllerOpen(i32 controllerIndex) override;
    void inputControllerClose(void* controllerPtr) override;
    u16 inputKeyboardGetMaxScancode() override;
    u32 inputMouseGetRelativeState(i32& x, i32& y) override;
    u32 inputMouseGetState(i32& x, i32& y) override;
    void inputMouseShowCursor(bool show) override;
    void inputMouseSetRelativeMode(bool isRelative) override;

protected:
    Window window { "Gaemi-01" };

};

}

#endif