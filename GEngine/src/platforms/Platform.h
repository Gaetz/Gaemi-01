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
#include "../Log.h"

namespace engine::input {
    enum class ControllerAxis;
}

namespace engine::platforms {

    class Platform {
    public:
        virtual ~Platform() = default;

        virtual bool init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) = 0;
        GAPI virtual void update(u32 dt) = 0;
        virtual void close() = 0;
        virtual bool pumpMessages() = 0;
        virtual void* getScreenSurface() = 0;

        virtual void* allocate(u64 size, bool aligned) = 0;
        virtual void free(void* block, bool aligned) = 0;
        virtual void* zeroMemory(void* block, u64 size) = 0;
        virtual void* copyMemory(void* dest, const void* source, u64 size) = 0;
        virtual void* setMemory(void* dest, i32 value, u64 size) = 0;

        virtual void consoleWrite(const string& message, u8 color) = 0;
        virtual void consoleWriteError(const string& message, u8 color) = 0;

        virtual u64 getAbsoluteTimeMs() = 0;
        virtual f64 getAbsoluteTimeSeconds() = 0;
        virtual void sleep(u64 ms) = 0;
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

    class NullPlatform : public Platform {
        bool init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) override { placeholderMessage(); return false; }
        GAPI void update(u32 dt) override { placeholderMessage(); }
        void close() override { placeholderMessage(); }
        bool pumpMessages() override { placeholderMessage(); return false; }
        void* getScreenSurface() override { placeholderMessage(); return nullptr; }

        void* allocate(u64 size, bool aligned) override { placeholderMessage(); return nullptr; }
        void free(void* block, bool aligned) override { placeholderMessage(); }
        void* zeroMemory(void* block, u64 size) override { placeholderMessage(); return nullptr; }
        void* copyMemory(void* dest, const void* source, u64 size) override { placeholderMessage(); return nullptr; }
        void* setMemory(void* dest, i32 value, u64 size) override { placeholderMessage(); return nullptr; }

        void consoleWrite(const string& message, u8 color) override { placeholderMessage(); }
        void consoleWriteError(const string& message, u8 color) override { placeholderMessage(); }

        u64 getAbsoluteTimeMs() override { placeholderMessage(); return 0; }
        f64 getAbsoluteTimeSeconds() override { placeholderMessage(); return 0; }
        void sleep(u64 ms) override { placeholderMessage(); }
        array<char, 19> getDate() override { placeholderMessage(); return {}; }

        const u8* inputKeyboardGetState(i32* keys) override { placeholderMessage(); return nullptr; }
        i32 inputMouseGetButtonMask(i32 button) override { placeholderMessage(); return 0; }
        i32 inputControllerGetButton(void* controllerPtr, i32 button) override { placeholderMessage(); return 0; }
        i32 inputControllerGetAxis(void* controllerPtr, engine::input::ControllerAxis axis) override { placeholderMessage(); return 0; }
        void* inputControllerOpen(i32 controllerIndex) override { placeholderMessage(); return nullptr; }
        void inputControllerClose(void* controllerPtr) override { placeholderMessage(); }
        u16 inputKeyboardGetMaxScancode() override { placeholderMessage(); return 0; }
        u32 inputMouseGetRelativeState(i32& x, i32& y) override { placeholderMessage(); return 0; }
        u32 inputMouseGetState(i32& x, i32& y) override { placeholderMessage(); return 0; }
        void inputMouseShowCursor(bool show) override { placeholderMessage(); }
        void inputMouseSetRelativeMode(bool show) override { placeholderMessage(); }

    private:
        void placeholderMessage() {
            LOG(LogLevel::Warning) << "Usage of placeholder memory service.";
        }
    };

}

#endif