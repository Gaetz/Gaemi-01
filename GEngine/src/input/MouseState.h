//
// Created by gaetz on 12/06/2021.
//

#ifndef INPUT_MOUSESTATE_H
#define INPUT_MOUSESTATE_H

#include "../math/Types.h"
#include "../Defines.h"

using engine::math::Vec2;
enum class ButtonState;

namespace engine::input {
class MouseState {
    friend class InputSystem;

public:
    const Vec2 &getPosition() const { return position; }

    bool getButtonValue(int button) const;

    ButtonState getButtonState(int button) const;

    const Vec2 &getScrollWheel() const { return scrollWheel; }

    bool isRelativeModeOn() const { return isRelativeMode; }

private:
    Vec2 position;
    u32 currentButtons;
    u32 previousButtons;
    Vec2 scrollWheel;
    bool isRelativeMode;
};
}
#endif //INPUT_MOUSESTATE_H
