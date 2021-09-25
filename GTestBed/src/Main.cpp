#include <Log.h>
#include "GameImpl.h"
#include "Engine.h"

using engine::EngineConfig;
using engine::Engine;
using engine::Log;
using game::GameImpl;

int main(int argc, char* argv[]) {
    // Init logging
    Log::restart();

    // Create game
    GameImpl game = GameImpl::instance();
    /*
    if (!engine::Entry::createGame(gameType)) {
        LOG(engine::LogLevel::Fatal) << "Could not create game.";
        return -1;
    }
    */

    // Config
    EngineConfig config {
            100, 100, 1280, 720,
            "Gaemi-01", false
    };

    // Engine
    Engine engine{config};
    engine.init(game);
    engine.run();
    engine.cleanup();

    return 0;
}