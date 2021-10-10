#include "Engine.h"
#include <fstream>
#include "Timer.h"


using engine::Engine;
using engine::input::InputState;
using std::array;
using engine::mem::MemoryManager;
using engine::mem::MemoryTag;

GAPI Engine::Engine(const EngineConfig& configP) :
    config { configP },
    inputSystem { configP.startWidth, configP.startHeight }
{}

void Engine::run() {
    Timer timer;
    state.game->load();
    while (state.isRunning) {
        u64 time = getAbsoluteTime();
        u32 dt = timer.computeDeltaTime(time);

        // Input
        const InputState& inputState = processInputs();
        state.game->setInputState(inputState);

        // Update loop
        update(dt);

        // Render
        draw(dt);

        // Frame delay is managed by the renderer,
        // which synchronizes with the monitor framerate.
    }
}

void Engine::init(Game& game, u64 sizeOfGameClass) {
    state.game = &game;

    bool platformIgnited = state.platform->init(config.name, config.startPositionX, config.startPositionY, config.startWidth, config.startHeight);
    if (platformIgnited) {
        LOG(LogLevel::Trace) << "Platform initialized";
    }

    state.memoryManager.init(state.platform);
    state.memoryManager.addAllocated(sizeOfGameClass, MemoryTag::Game);

    bool rendererIgnited = renderer.init(config.name, config.startWidth, config.startHeight);
    if (rendererIgnited) {
        LOG(LogLevel::Trace) << "Renderer initialized";
    }

    bool eventsIgnited = state.eventManager.init();
    if (eventsIgnited) {
        LOG(LogLevel::Trace) << "Events initialized";
    }
    state.eventManager.subscribe(EventCode::ApplicationQuit, nullptr, &onEngineEvent);

    bool inputsIngnited = inputSystem.init();
    if (inputsIngnited) {
        LOG(LogLevel::Trace) << "Inputs initialized";
    }

    state.isInitialized = platformIgnited && rendererIgnited && eventsIgnited && inputsIngnited;

    if(!state.isInitialized) {
        LOG(LogLevel::Fatal) << "Subsystem not initialized. Application will shut down.";
        renderer.close();
        state.eventManager.unsubscribe(EventCode::ApplicationQuit, nullptr, &onEngineEvent);
        state.game->close();
        inputSystem.close();
        state.eventManager.close();
        state.platform->close();
        state.memoryManager.close();
    } else {
        state.isRunning = true;
        state.isPaused = false;
    }
}

void Engine::close() {
    if (state.isInitialized) {
        renderer.close();
        state.eventManager.unsubscribe(EventCode::ApplicationQuit, nullptr, &onEngineEvent);
        state.game->close();
        inputSystem.close();
        state.eventManager.close();
        state.platform->close();
        state.memoryManager.close();
    }
}

// TODO Get rid of SDL for inputs, move it to platform
InputState engine::Engine::processInputs() {
    inputSystem.preUpdate();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        inputSystem.processEvent(event);
    }
    inputSystem.update();

    return inputSystem.getInputState();
}

void Engine::update(u32 dt) {
    state.platform->update(dt);
    state.game->update(dt);
}

u64 Engine::getAbsoluteTime() const {
    return state.platform->getAbsoluteTimeMs();
}

f64 Engine::getAbsoluteTimeSeconds() const {
    return state.platform->getAbsoluteTimeSeconds();
}

void Engine::sleep(u64 ms) const {
    state.platform->sleep(ms);
}


std::array<char, 19> engine::Engine::getDate() {
    return state.platform->getDate();
}

bool engine::Engine::handleEngineEvent(EventCode code, void* sender, void* listenerInstance, EventContext context) {
    switch (code) {
        case EventCode::ApplicationQuit:
            LOG(LogLevel::Trace) << "EventCode::ApplicationQuit received, closing application.";
            state.isRunning = false;
            return true;
        default:
            break;
    }
    return false;
}

void Engine::draw(u32 dt) {
    // Game update
    state.game->draw();

    render::RenderPacket renderPacket;
    renderPacket.dt = dt;
    renderer.drawFrame(renderPacket);
}
