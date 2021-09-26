//
// Created by gaetz on 26/09/2021.
//

#include "Entry.h"

using engine::Entry;

int Entry::start(EngineConfig& config, game::Game& game) {
    // Init logging
    Log::restart();

    // Engine
    Engine engine{config};
    engine.init(game);
    engine.run();
    engine.cleanup();
    return 0;
}