//
// Created by gaetz on 12/06/2021.
//

#include "MouseState.h"
#include <SDL2/SDL_mouse.h>
#include "InputManager.h"
#include "../Locator.h"

using engine::input::MouseState;

bool MouseState::getButtonValue(i32 button) const {
    i32 mask = Locator::platform().inputMouseGetButtonMask(button);
    return (mask & currentButtons) == 1;
}

ButtonState MouseState::getButtonState(i32 button) const {
    i32 mask = Locator::platform().inputMouseGetButtonMask(button);
    if ((mask & previousButtons) == 0) {
        if ((mask & currentButtons) == 0) {
            return ButtonState::None;
        } else {
            return ButtonState::Pressed;
        }
    } else {
        if ((mask & currentButtons) == 0) {
            return ButtonState::Released;
        } else {
            return ButtonState::Held;
        }
    }
}
