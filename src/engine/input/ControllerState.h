//
// Created by gaetz on 12/06/2021.
//

#ifndef INPUT_CONTROLLERSTATE_H
#define INPUT_CONTROLLERSTATE_H

#include <SDL2/SDL_gamecontroller.h>
#include "../math/Types.h"

using engine::math::Vec2;
enum class ButtonState;

namespace engine::input {
class ControllerState {
public:
    friend class InputSystem;

    ControllerState();

    bool getButtonValue(SDL_GameControllerButton button) const;

    ButtonState getButtonState(SDL_GameControllerButton button) const;

    const Vec2& getLeftStick() const { return leftStick; }

    const Vec2& getRightStick() const { return rightStick; }

    float getLeftTrigger() const { return leftTrigger; }

    float getRightTrigger() const { return rightTrigger; }

    bool getIsConnected() const { return isConnected; }


private:
    Uint8 currentButtons[SDL_CONTROLLER_BUTTON_MAX];
    Uint8 previousButtons[SDL_CONTROLLER_BUTTON_MAX];
    Vec2 leftStick;
    Vec2 rightStick;
    float leftTrigger;
    float rightTrigger;
    bool isConnected;
};
}


#endif //INPUT_CONTROLLERSTATE_H
