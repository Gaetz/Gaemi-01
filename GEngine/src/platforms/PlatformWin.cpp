#include "PlatformWin.h"
#include <array>
#include <time.h>

using std::array;

using engine::platforms::PlatformWin;

PlatformWin::~PlatformWin() {

}

bool PlatformWin::init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) {
    SDL_Init(SDL_INIT_VIDEO);
    return window.init(x, y, width, height, false);
}

void PlatformWin::update(u32 dt) {
    window.updateFpsCounter(dt);
}

void PlatformWin::shutdown() {
    window.cleanup();
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

void PlatformWin::sleep(u32 ms) {
    SDL_Delay(ms);
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
