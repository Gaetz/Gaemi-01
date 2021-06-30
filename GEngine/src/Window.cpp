//
// Created by gaetz on 08/06/2021.
//

#include <utility>

#include "Window.h"
#include "Log.h"

using engine::Window;

Window::Window(string titleP): title(std::move(titleP)),
                                     previousSeconds(0),
                                     currentSeconds(0),
                                     frameCount(0) {

}

Window::~Window() {
    LOG(LogLevel::Info) << "Bye :)";
}

bool Window::init(int width, int height, bool isFullscreen) {

    SDL_WindowFlags flags = SDL_WINDOW_VULKAN;
    window = std::unique_ptr<SDL_Window, SdlWindowDestroyer>(
            SDL_CreateWindow(title.c_str(),
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             width, height, flags)
    );

    if (!window) return false;

    LOG(LogLevel::Info) << "Window initialised";
    return true;
}

void Window::cleanup() {

}

void Window::updateFpsCounter(long dt) {
    double elapsedSeconds;

    currentSeconds += dt / 1000.0;
    elapsedSeconds = currentSeconds - previousSeconds;
    /* limit text updates to 4 per second */
    if (elapsedSeconds > 0.25)
    {
        previousSeconds = currentSeconds;
        char tmp[128];
        double fps = (double)frameCount / elapsedSeconds;
#if GPLATFORM_WINDOWS
        sprintf_s(tmp, "%s @ fps: %.2f", title.c_str(), fps);
#else
        sprintf(tmp, "%s @ fps: %.2f", title.c_str(), fps);
#endif
        SDL_SetWindowTitle(window.get(), tmp);
        frameCount = 0;
    }
    frameCount++;
}
