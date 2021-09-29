//
// Created by gaetz on 26/09/2021.
//

#include "Entry.h"

using engine::Entry;

int Entry::start(EngineConfig& config, game::Game& game, u64 sizeOfGameClass) {
    // Init logging
    Log::restart();

    // Engine
    Engine engine{config};
    engine.init(game, sizeOfGameClass);
    engine.run();
    engine.cleanup();
    return 0;
}