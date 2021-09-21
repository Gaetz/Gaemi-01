#include "PlatformWin.h"

using engine::platforms::PlatformWin;

PlatformWin::~PlatformWin() {

}

b8 PlatformWin::init(const string& applicationName, i32 x, i32 y, i32 width, i32 height) {
    return window.init(x, y, width, height, false);
}

void PlatformWin::update(u64 dt) {
    window.updateFpsCounter(dt);
}

void PlatformWin::shutdown() {
    window.cleanup();
}

b8 PlatformWin::pumpMessages() {
    return false;
}

void* PlatformWin::getScreenSurface() {
    return window.get();
}

void* PlatformWin::allocate(u64 size, b8 aligned) {
    return nullptr;
}


void PlatformWin::free(void* block, b8 aligned) {

}

void* PlatformWin::zeroMemory(void* block, u64 size) {
    return nullptr;
}

void* PlatformWin::copyMemory(void* dest, const void* source, u64 size) {
    return nullptr;
}

void* PlatformWin::setMemory(void* dest, i32 value, u64 size){
    return nullptr;
}

void PlatformWin::consoleWrite(const string& message, u8 color) {

}

void PlatformWin::consoleWriteError(const string& message, u8 color) {

}


f64 PlatformWin::getAbsoluteTime() {
    return 0.0f;
}

void PlatformWin::sleep(u64 ms) {

}