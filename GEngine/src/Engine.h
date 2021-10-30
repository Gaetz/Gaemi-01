//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <unordered_map>
#include <array>
#include <functional>
#include <GameObject.h>

#include "Defines.h"
#include "Game.h"
#include "platforms/PlatformWin.h"
#include "input/InputManager.h"
#include "mem/MemoryManager.h"
#include "EventManager.h"
#include "render/RendererFrontEnd.h"
#include "asset/AssetManager.h"

using std::vector;
using std::unordered_map;
using std::array;
using std::unique_ptr;
using std::make_unique;

using engine::input::InputManager;
using engine::platforms::Platform;
using engine::platforms::PlatformWin;
using game::Game;
using engine::mem::MemoryManager;
using engine::EventManager;
using engine::render::RendererFrontEnd;
using engine::asset::AssetManager;

namespace engine {

struct EngineConfig {
    i16 startPositionX;
    i16 startPositionY;
    u16 startWidth;
    u16 startHeight;
    string name;
    bool fullscreenMode;
};

struct EngineState {
    bool isInitialized { false };
    bool isRunning { false };
    bool isPaused { false };
    Game* game;
    MemoryManager memoryManager;
    EventManager eventManager;

    // Platform
    #ifdef GPLATFORM_WINDOWS
    Platform* platform = new PlatformWin();
    #else
    Platform* platform { nullptr };
    // No implementation, won't compile
    #endif
};

class Engine {
private:
    EngineState state;
    EngineConfig config;
    static RendererFrontEnd renderer;
    AssetManager assets { renderer };

public:
    GAPI explicit Engine(const EngineConfig& configP);
    GAPI ~Engine() = default;

    InputManager inputSystem;

    // Initializes everything in the engine
    void init(Game& game, u64 sizeOfGameClass);

    // Run the engine
    void run();

    // Shuts down the engine
    void close();

    // Process engine inputs
    input::InputState processInputs();

    // Update loop
    void update(u32 dt);

    // Draw loop
    void draw(u32 dt);

    // Add a game object in the scene
    static GAPI void addToScene(engine::render::vk::GameObject& gameObject);

    // Events
    bool handleEngineEvent(EventCode code, void* sender, void* listenerInstance, EventContext context);

    EventCallback onEngineEvent = [this](EventCode code, void* sender, void* listInst, EventContext context) {
        return this->handleEngineEvent(code, sender, listInst, context);
    };
};

}
#endif //ENGINE_H
