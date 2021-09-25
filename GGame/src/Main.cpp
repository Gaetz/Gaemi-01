#include <Log.h>
#include "GameImpl.h"
#include "Engine.h"

using engine::EngineConfig;
using engine::Engine;
using engine::Log;
using game::GameImpl;

void createGame(GameType &gameType) {
    GameImpl& game = GameImpl::instance();
    gameType.onLoad = std::bind(&GameImpl::load, game);
    gameType.onUpdate = std::bind(&GameImpl::update, game, std::placeholders::_1);
    gameType.onDraw = std::bind(&GameImpl::draw, game);
    gameType.onCleanup = std::bind(&GameImpl::cleanup, game);
}

int main(int argc, char* argv[]) {
    // Init logging
    Log::restart();

    // Create game
    GameType gameType;
    createGame(gameType);
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
    engine.init(gameType);
    engine.run();
    engine.cleanup();

    return 0;
}