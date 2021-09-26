#include "GameImpl.h"
#include "Entry.h"

using engine::EngineConfig;
using game::GameImpl;

int main(int argc, char* argv[]) {
    // Config
    EngineConfig config {
            100, 100, 1280, 720,
            "Gaemi-01", false
    };
    // Create game
    GameImpl game = GameImpl::instance();
    // Startup
    engine::Entry::start(config, game);

    return 0;
}