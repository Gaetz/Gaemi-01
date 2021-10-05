//
// Created by gaetz on 12/06/2021.
//

#ifndef INPUT_CONTROLLERSTATE_H
#define INPUT_CONTROLLERSTATE_H

#include "../math/Types.h"
#include "../Defines.h"

using engine::math::Vec2;
enum class ButtonState;

namespace engine::input {

    /**
     * Controller buttons axis, actually use SDL mapping (SDL_GameControllerAxis)
     */
    enum class ControllerAxis {
        Invalid = -1,
        LeftX,
        LeftY,
        RightX,
        RightY,
        TriggerLeft,
        TriggerRight,
        Max
    };

    /**
     * Controller buttons ids, actually use SDL mapping (SDL_GameControllerButton)
     */
    enum class ControllerButton {
        INVALID = -1,
        A,
        B,
        X,
        Y,
        Back,
        Guide,
        Start,
        LeftStick,
        RightStick,
        LedtShoulder,
        RightShoulder,
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
        Misc1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button */
        Paddle1,  /* Xbox Elite paddle P1 */
        Paddle2,  /* Xbox Elite paddle P3 */
        Paddle3,  /* Xbox Elite paddle P2 */
        Paddle4,  /* Xbox Elite paddle P4 */
        TouchPad, /* PS4/PS5 touchpad button */
        Max
    };

    class ControllerState {
    public:
        friend class InputSystem;

        ControllerState() = default;

        bool getButtonValue(ControllerButton button) const;

        ButtonState getButtonState(ControllerButton button) const;

        const Vec2& getLeftStick() const { return leftStick; }

        const Vec2& getRightStick() const { return rightStick; }

        f32 getLeftTrigger() const { return leftTrigger; }

        f32 getRightTrigger() const { return rightTrigger; }

        bool getIsConnected() const { return isConnected; }


    private:
        u8 currentButtons[static_cast<int>(ControllerButton::Max)];
        u8 previousButtons[static_cast<int>(ControllerButton::Max)];
        Vec2 leftStick { Vec2() };
        Vec2 rightStick { Vec2() };
        f32 leftTrigger { 0.0f };
        f32 rightTrigger { 0.0f };
        bool isConnected { false };
    };
}


#endif //INPUT_CONTROLLERSTATE_H
