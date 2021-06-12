#include "engine/Engine.h"
#include "Log.h"

using engine::Engine;

LogConfig LOG_CONFIG {};

int main(int argc, char* argv[])
{
    // Init logging
    LOG_CONFIG.reportingLevel = static_cast<int>(LogLevel::Debug);
    LOG_CONFIG.restart = true;
    if (LOG_CONFIG.restart) {
        Log::restart();
    }

    // Engine
    Engine engine;
    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}