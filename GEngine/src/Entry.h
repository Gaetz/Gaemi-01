//
// Created by gaetz on 26/09/2021.
//

#ifndef ENTRY_H
#define ENTRY_H

#include "Engine.h"
#include "Log.h"

using engine::Engine;
using engine::Log;

namespace engine {
    class Entry {
    public:
        GAPI static int start(EngineConfig& config, game::Game& game);
    };
}

#endif //ENTRY_H
