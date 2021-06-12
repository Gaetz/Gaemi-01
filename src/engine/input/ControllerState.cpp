//
// Created by gaetz on 12/06/2021.
//

#include "ControllerState.h"
#include "InputSystem.h"

using engine::input::ControllerState;

ControllerState::ControllerState() :
        leftStick(Vec2()),
        rightStick(Vec2()),
        leftTrigger(0.0f),
        rightTrigger(0.0f),
        isConnected(false) {

}


bool ControllerState::getButtonValue(SDL_GameControllerButton button) const {
    return currentButtons[button] == 1;
}

ButtonState ControllerState::getButtonState(SDL_GameControllerButton button) const {
    if (previousButtons[button] == 0) {
        if (currentButtons[button] == 0) {
            return ButtonState::None;
        } else {
            return ButtonState::Pressed;
        }
    } else {
        if (currentButtons[button] == 0) {
            return ButtonState::Released;
        } else {
            return ButtonState::Held;
        }
    }
}
