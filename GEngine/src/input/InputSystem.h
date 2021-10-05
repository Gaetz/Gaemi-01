//
// Created by gaetz on 12/06/2021.
//

#ifndef INPUTSYSTEM_H
#define INPUTSYSTEM_H

#include <SDL2/SDL.h>

#include "../math/Types.h"
#include "KeyboardState.h"
#include "MouseState.h"
#include "ControllerState.h"
#include "../Defines.h"

enum class ButtonState {
    None,
    Pressed,
    Released,
    Held
};

namespace engine::input {

struct InputState {
    KeyboardState keyboard;
    MouseState mouse;
    ControllerState controller;
};

class InputSystem {
public:
    InputSystem(u32 windowWidthP, u32 windowHeightP);
    InputSystem() = delete;

    bool init();

    void close();

    const InputState& getInputState() const { return inputState; }

    // Returns isRunning status
    void processEvent(SDL_Event& event);

    void preUpdate();

    void update();

    bool getIsCursorDisplayed() const { return isCursorDisplayed; }

    void setMouseCursor(bool isCursorDisplayedP);

    void setMouseRelativeMode(bool isMouseRelativeOnP);

private:
    u32 windowWidth { 1280 };
    u32 windowHeight { 720 };
    InputState inputState {};
    bool isCursorDisplayed { false };
    void* controllerPtr { nullptr };

    f32 filter1D(int input);
    Vec2 filter2D(int inputX, int inputY);
};

constexpr i32 CONTROLLER_DEAD_ZONE_1D = 250;
constexpr f32 CONTROLLER_DEAD_ZONE_2D = 8000.0f;
constexpr i32 CONTROLLER_MAX_VALUE = 30000;

}

#endif //INPUTSYSTEM_H
