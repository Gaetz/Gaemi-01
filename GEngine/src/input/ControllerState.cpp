//
// Created by gaetz on 12/06/2021.
//

#include "ControllerState.h"
#include "InputSystem.h"

using engine::input::ControllerState;

bool ControllerState::getButtonValue(ControllerButton button) const {
    i32 buttonIndex = static_cast<i32>(button);
    return currentButtons[buttonIndex] == 1;
}

ButtonState ControllerState::getButtonState(ControllerButton button) const {
    i32 buttonIndex = static_cast<i32>(button);
    if (previousButtons[buttonIndex] == 0) {
        if (currentButtons[buttonIndex] == 0) {
            return ButtonState::None;
        } else {
            return ButtonState::Pressed;
        }
    } else {
        if (currentButtons[buttonIndex] == 0) {
            return ButtonState::Released;
        } else {
            return ButtonState::Held;
        }
    }
}
