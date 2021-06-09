
#include "Engine.h"

#include "Types.h"
#include "VkInit.h"
#include "Timer.h"


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

    Timer timer;

    game.load();
    while (game.isRunning)
    {
        uint32_t dt = timer.computeDeltaTime();
        window.updateFpsCounter(dt);

        game.handleInputs();
        game.update(dt);
        draw();
        game.draw();

        // Delay frame if game runs too fast
        timer.delayTime();
    }
}
