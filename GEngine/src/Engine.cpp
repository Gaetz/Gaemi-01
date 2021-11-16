#include "Engine.h"
#include <fstream>
#include <imgui.h>
#include "Timer.h"
#include "Locator.h"


using engine::Engine;
using engine::input::InputState;
using std::array;
using engine::mem::MemoryManager;
using engine::mem::MemoryTag;

RendererFrontEnd Engine::renderer {};

GAPI Engine::Engine(const EngineConfig& configP) :
    config { configP },
    inputSystem { configP.startWidth, configP.startHeight }
{}

void Engine::run() {
    Timer timer;
    state.game->load();
    while (state.isRunning) {
        u64 time = Locator::platform().getAbsoluteTimeMs();
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

    bool assetsIgnited = assets.init();
    if (assetsIgnited) {
        LOG(LogLevel::Trace) << "Assets initialized";
    }

    bool rendererIgnited = renderer.init(config.name, config.startWidth, config.startHeight);
    if (rendererIgnited) {
        LOG(LogLevel::Trace) << "Renderer initialized";
    }

    bool eventsIgnited = state.eventManager.init();
    if (eventsIgnited) {
        LOG(LogLevel::Trace) << "Events initialized";
    }
    state.eventManager.subscribe(EventCode::ApplicationQuit, nullptr, &onEngineEvent);

    bool inputsIgnited = inputSystem.init();
    if (inputsIgnited) {
        LOG(LogLevel::Trace) << "Inputs initialized";
    }

    state.isInitialized = platformIgnited && rendererIgnited && eventsIgnited && inputsIgnited && assetsIgnited;

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
        // Assets should be closed before renderer, to free graphics assets memory
        assets.close();
        // Then we can free the remaining graphics memory
        renderer.close();
        // And finally other managers
        state.game->close();
        state.eventManager.unsubscribe(EventCode::ApplicationQuit, nullptr, &onEngineEvent);
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
        Locator::platform().imGuiProcessEvent(&event);
        inputSystem.processEvent(event);
    }
    inputSystem.update();

    return inputSystem.getInputState();
}

void Engine::update(u32 dt) {
    state.platform->update(dt);
    state.game->update(dt);
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
    // ImGUI
    Locator::platform().imGuiNewFrame();
    ImGui::NewFrame();
    // vvvvv ImGui code should go here vvvvv
    {
        //ImGui::ShowDemoWindow();
    }
    // ^^^^^ ImGui code should go there ^^^^^
    ImGui::Render();

    // Game draw
    state.game->draw();

    render::RenderPacket renderPacket {};
    renderPacket.dt = dt;
    renderer.drawFrame(renderPacket);
}

void Engine::addToScene(engine::render::vk::GameObject &gameObject) {
    renderer.addToScene(gameObject);
}
