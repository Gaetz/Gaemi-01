#include <Log.h>
#include <Timer.h>
#include <input/InputSystem.h>
#include <Engine.h>

using engine::Engine;
using engine::Timer;
using engine::input::InputState;

void run(Engine& engine);

int main(int argc, char* argv[])
{
    // Init logging
    engine::Log::restart();

    // Engine
    Engine engine;
    engine.init();
    run(engine);
    engine.cleanup();

    return 0;
}

void run(Engine& engine) {

    Timer timer;

    //game.load();
    while (engine.isRunning) {
        uint32_t dt = timer.computeDeltaTime();
        engine.window.updateFpsCounter(dt);

        const InputState inputState = engine.processInputs();
        //game.update(dt);
        engine.draw();

        // Frame delay is managed by the renderer,
        // which synchronizes with the monitor framerate.
    }
}