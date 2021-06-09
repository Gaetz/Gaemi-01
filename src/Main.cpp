#include "engine/Engine.h"
#include "engine/Log.h"

LogConfig LOG_CONFIG {};

int main(int argc, char* argv[])
{
    // Init logging
    LOG_CONFIG.reporting_level = Debug;
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