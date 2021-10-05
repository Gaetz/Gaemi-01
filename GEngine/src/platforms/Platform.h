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
#include "../input/KeyboardState.h"
#include "../input/ControllerState.h"

namespace engine::platforms {

class Platform {
public:
    virtual ~Platform() = default;

    virtual bool init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) = 0;
    GAPI virtual void update(u32 dt) = 0;
    virtual void close() = 0;
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

    virtual const u8* inputKeyboardGetState(i32* keys) = 0;
    virtual i32 inputMouseGetButtonMask(i32 button) = 0;
    virtual i32 inputControllerGetButton(void* controllerPtr, i32 button) = 0;
    virtual i32 inputControllerGetAxis(void* controllerPtr, engine::input::ControllerAxis axis) = 0;
    virtual void* inputControllerOpen(i32 controllerIndex) = 0;
    virtual void inputControllerClose(void* controllerPtr) = 0;
    virtual u16 inputKeyboardGetMaxScancode() = 0;
    virtual u32 inputMouseGetRelativeState(i32& x, i32& y) = 0;
    virtual u32 inputMouseGetState(i32& x, i32& y) = 0;
    virtual void inputMouseShowCursor(bool show) = 0;
    virtual void inputMouseSetRelativeMode(bool show) = 0;

};

}

#endif