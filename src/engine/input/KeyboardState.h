//
// Created by gaetz on 12/06/2021.
//

#ifndef INPUT_KEYBOARDSTATE_H
#define INPUT_KEYBOARDSTATE_H

#include <SDL2/SDL_keyboard.h>
enum class ButtonState;

namespace engine::input {
class KeyboardState {
    friend class InputSystem;

public:
    bool getKeyValue(SDL_Scancode key) const;
    ButtonState getKeyState(SDL_Scancode key) const;

private:
    const Uint8* currentState;
    Uint8 previousState[SDL_NUM_SCANCODES];
};
}

#endif //INPUT_KEYBOARDSTATE_H
