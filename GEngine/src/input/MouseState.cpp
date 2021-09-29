//
// Created by gaetz on 12/06/2021.
//

#include "MouseState.h"
#include <SDL2/SDL_mouse.h>
#include "InputSystem.h"

using engine::input::MouseState;

bool MouseState::getButtonValue(int button) const {
    return (SDL_BUTTON(button) & currentButtons) == 1;
}

ButtonState MouseState::getButtonState(int button) const {
    i32 mask = SDL_BUTTON(button);
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
