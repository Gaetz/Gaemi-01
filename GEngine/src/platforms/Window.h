//
// Created by gaetz on 08/06/2021.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <memory>
#include <string>

#include "../Defines.h"

using std::string;
using std::unique_ptr;

namespace engine::platforms {

struct SdlWindowDestroyer {
    void operator()(SDL_Window* window) const {
        SDL_DestroyWindow(window);
    }
};

class Window {
public:
    explicit Window(string titleP);

    ~Window();

    bool init(i32 x, i32 y, i32 width, i32 height, bool isFullscreen);

    void updateFpsCounter(u64 dt);

    void cleanup();

    SDL_Window* get() { return window.get(); }

private:
    unique_ptr<SDL_Window, SdlWindowDestroyer> window;
    string title;
    double previousSeconds;
    double currentSeconds;
    int frameCount;
};

}

#endif //WINDOW_H
