
#include "Engine.h"

#include "Types.h"
#include "VkInit.h"


void Engine::init() {
    SDL_Init(SDL_INIT_VIDEO);

    isInitialized = window.init(windowExtent.width, windowExtent.height, false);
}

void Engine::cleanup() {
    window.cleanup();
    SDL_Quit();
}

void Engine::draw() {

}

void Engine::run() {
    SDL_Event e;
    bool quit = false;

    //main loop
    while (!quit)
    {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            //close the window when user clicks the X button or alt-f4s
            if (e.type == SDL_QUIT) quit = true;
        }

        draw();
    }
}
