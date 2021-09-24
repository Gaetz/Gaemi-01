#include <Log.h>
#include <Engine.h>

using engine::Engine;
using engine::Log;

int main(int argc, char* argv[])
{
    // Init logging
    Log::restart();

    // Engine
    Engine engine { config };
    engine.init();
    engine.run();
    engine.cleanup();

    return 0;
}

