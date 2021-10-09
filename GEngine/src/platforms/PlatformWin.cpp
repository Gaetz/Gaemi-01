#include "PlatformWin.h"
#include "../input/ControllerState.h"
#include "../Locator.h"
#include <array>
#include <time.h>
#include <SDL2/SDL_vulkan.h>

using std::array;

using engine::platforms::PlatformWin;

bool PlatformWin::init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) {
    SDL_Init(SDL_INIT_VIDEO);
    Locator::provide(this);
    return window.init(x, y, width, height, false);
}

void PlatformWin::update(u32 dt) {
    window.updateFpsCounter(dt);
}

void PlatformWin::close() {
    window.cleanup();
    LOG(LogLevel::Trace) << "Bye :)";
    SDL_Quit();
}

bool PlatformWin::pumpMessages() {
    return false;
}

void* PlatformWin::getScreenSurface() {
    return window.get();
}

void* PlatformWin::allocate(u64 size, bool aligned) {
    return malloc(size);
}

void PlatformWin::free(void* block, bool aligned) {
    ::free(block);
}

void* PlatformWin::zeroMemory(void* block, u64 size) {
    return memset(block, 0, size);
}

void* PlatformWin::copyMemory(void* dest, const void* source, u64 size) {
    return memcpy(dest, source, size);
}

void* PlatformWin::setMemory(void* dest, i32 value, u64 size){
    return memset(dest, value, size);
}

void PlatformWin::consoleWrite(const string& message, u8 color) {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    // Fatal, Error, Warn, Info, Debug, Trace
    static array<u8, 6> levels { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(consoleHandle, levels[color]);
    OutputDebugStringA(message.c_str());
    LPDWORD numberWritten = 0;
    WriteConsoleA(consoleHandle, message.c_str(), (DWORD)message.length(), numberWritten, 0);
}

void PlatformWin::consoleWriteError(const string& message, u8 color) {
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    // Fatal, Error, Warn, Info, Debug, Trace
    static array<u8, 6> levels { 64, 4, 6, 2, 1, 8 };
    SetConsoleTextAttribute(consoleHandle, levels[color]);
    OutputDebugStringA(message.c_str());
    LPDWORD numberWritten = 0;
    WriteConsoleA(consoleHandle, message.c_str(), (DWORD)message.length(), numberWritten, 0);
}

u64 PlatformWin::getAbsoluteTimeMs() {
    return static_cast<u64>(SDL_GetTicks());
}

f64 PlatformWin::getAbsoluteTimeSeconds() {
    return static_cast<f64>(SDL_GetTicks()) / 1000.0f;
}

void PlatformWin::sleep(u64 ms) {
    SDL_Delay(static_cast<u32>(ms));
}

array<char, 19> engine::platforms::PlatformWin::getDate() {
    time_t now;
    array<char, 19> date {};
    struct tm timeInfo {};
    time(&now);
    localtime_s(&timeInfo, &now);
    strftime(date.data(), 19, "%y-%m-%d %H:%M:%S", &timeInfo);
    return date;

    /** LINUX
    struct tm* timeInfo;
    timeInfo = localtime(&now);
    strftime(date, 19, "%y-%m-%d %H:%M:%S", timeInfo);
    */
}

const u8 *engine::platforms::PlatformWin::inputKeyboardGetState(i32 *keys) {
    return SDL_GetKeyboardState(keys);
}

i32 engine::platforms::PlatformWin::inputMouseGetButtonMask(i32 button) {
    return SDL_BUTTON(button);
}

i32 engine::platforms::PlatformWin::inputControllerGetButton(void *controllerPtr, i32 button) {
    return SDL_GameControllerGetButton(static_cast<SDL_GameController*>(controllerPtr),
                                     static_cast<SDL_GameControllerButton>(button));
}

i32 engine::platforms::PlatformWin::inputControllerGetAxis(void *controllerPtr, engine::input::ControllerAxis axis) {
    return SDL_GameControllerGetAxis(static_cast<SDL_GameController*>(controllerPtr),
                                     static_cast<SDL_GameControllerAxis>(axis));
}

void* engine::platforms::PlatformWin::inputControllerOpen(i32 controllerIndex) {
    return SDL_GameControllerOpen(controllerIndex);
}

void engine::platforms::PlatformWin::inputControllerClose(void *controllerPtr) {
    SDL_GameControllerClose(static_cast<SDL_GameController*>(controllerPtr));
}

u16 engine::platforms::PlatformWin::inputKeyboardGetMaxScancode() {
    return SDL_NUM_SCANCODES;
}

u32 engine::platforms::PlatformWin::inputMouseGetRelativeState(i32 &x, i32 &y) {
    return SDL_GetRelativeMouseState(&x, &y);;
}

u32 engine::platforms::PlatformWin::inputMouseGetState(i32 &x, i32 &y) {
    return SDL_GetMouseState(&x, &y);;
}

void engine::platforms::PlatformWin::inputMouseShowCursor(bool show) {
    if (show) {
        SDL_ShowCursor(SDL_TRUE);
    } else {
        SDL_ShowCursor(SDL_FALSE);
    }
}

void engine::platforms::PlatformWin::inputMouseSetRelativeMode(bool isRelative) {
    if (isRelative) {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    } else {
        SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

void engine::platforms::PlatformWin::rendererSetupVulkanSurface(const VkInstance& instance, VkSurfaceKHR *surface) {
    SDL_Vulkan_CreateSurface((SDL_Window*)getScreenSurface(), instance, surface);
}

