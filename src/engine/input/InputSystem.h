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
    InputSystem(uint32_t windowWidthP, uint32_t windowHeightP);
    InputSystem() = delete;

    bool init();

    void cleanup();

    const InputState getInputState() const { return inputState; }

    bool processEvent(SDL_Event& event); // Returns isRunning status
    void preUpdate();

    void update();

    bool getIsCursorDisplayed() const { return isCursorDisplayed; }

    void setMouseCursor(bool isCursorDisplayedP);

    void setMouseRelativeMode(bool isMouseRelativeOnP);

private:
    uint32_t windowWidth;
    uint32_t windowHeight;
    InputState inputState;
    bool isCursorDisplayed;
    SDL_GameController* controller;

    float filter1D(int input);

    Vec2 filter2D(int inputX, int inputY);
};

constexpr int CONTROLLER_DEAD_ZONE_1D = 250;
constexpr float CONTROLLER_DEAD_ZONE_2D = 8000.0f;
constexpr int CONTROLLER_MAX_VALUE = 30000;

}

#endif //INPUTSYSTEM_H
