//
// Created by gaetz on 26/09/2021.
//

#include "Entry.h"
#include "../../externals/easy_profiler/include/easy/profiler.h"

using engine::Entry;

int Entry::start(EngineConfig& config, game::Game& game, u64 sizeOfGameClass) {

    // Init profiler
    EASY_MAIN_THREAD;
    EASY_PROFILER_ENABLE;

    // Init logging
    Log::restart();

    // Engine
    Engine engine{config};
    engine.init(game, sizeOfGameClass);
    engine.run();
    engine.close();
    return 0;
}