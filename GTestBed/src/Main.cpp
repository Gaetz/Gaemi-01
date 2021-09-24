#include <Log.h>
#include <Engine.h>

using engine::Log;
using engine::EngineConfig;
using engine::Engine;


int main(int argc, char* argv[]) {
    // Init logging
    Log::restart();

    // Config
    EngineConfig config {
        100, 100, 1280, 720,
        "Gaemi-01", false
    };

    // Engine
    Engine engine { config };
    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}
