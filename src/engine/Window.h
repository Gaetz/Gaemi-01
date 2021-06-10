//
// Created by gaetz on 08/06/2021.
//

#ifndef WINDOW_H
#define WINDOW_H

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#elif _WIN32
#include <SDL.h>
#include <SDL_vulkan.h>
#endif

#include <memory>
using std::unique_ptr;

#include <string>
using std::string;

struct SdlWindowDestroyer
{
    void operator()(SDL_Window *window) const
    {
        SDL_DestroyWindow(window);
    }
};

class Window {
public:
    explicit Window(string titleP);
    ~Window();

    bool init(int width, int height, bool isFullscreen);
    void updateFpsCounter(long dt);
    void cleanup();
    SDL_Window* get() { return window.get(); }

private:
    unique_ptr<SDL_Window, SdlWindowDestroyer> window;
    string title;
    double previousSeconds;
    double currentSeconds;
    int frameCount;
};


#endif //WINDOW_H
