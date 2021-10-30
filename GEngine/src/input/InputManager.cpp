//
// Created by gaetz on 12/06/2021.
//

#include "InputManager.h"
#include "../math/Functions.h"
#include "../Locator.h"

using engine::input::InputManager;

InputManager::InputManager(u32 windowWidthP, u32 windowHeightP) :
        windowWidth {windowWidthP},
        windowHeight {windowHeightP} {

}

bool InputManager::init() {
    auto& memory = Locator::memory();
    auto& platform = Locator::platform();
    // Keyboard
    // Assign current state pointer
    inputState.keyboard.currentState = platform.inputKeyboardGetState(nullptr);
    // Clear previous state memory
    u16 maxScancode = platform.inputKeyboardGetMaxScancode();
    memory.set(inputState.keyboard.previousState, 0, maxScancode);

    // Mouse (just set everything to 0)
    inputState.mouse.currentButtons = 0;
    inputState.mouse.previousButtons = 0;

    // Get the connected controllerPtr, if it exists
    controllerPtr = platform.inputControllerOpen(0);
    // Initialize controllerPtr state
    inputState.controller.isConnected = (controllerPtr != nullptr);
    i16 maxControllerButton = static_cast<i16>(ControllerButton::Max);
    memory.set(inputState.controller.currentButtons, 0, maxControllerButton);
    memory.set(inputState.controller.previousButtons, 0, maxControllerButton);

    return true;
}

void InputManager::close() {
    if (controllerPtr != nullptr) {
        Locator::platform().inputControllerClose(controllerPtr);
    }
}

void InputManager::processEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            Locator::events().fire(EventCode::ApplicationQuit, nullptr, {});
            break;
        case SDL_MOUSEWHEEL:
            inputState.mouse.scrollWheel = Vec2(
                    static_cast<float>(event.wheel.x),
                    static_cast<float>(event.wheel.y));
            break;
        default:
            break;
    }
}

void InputManager::preUpdate() {
    auto& platform = Locator::platform();
    auto& memory = Locator::memory();
    // Copy current state to previous
    // Keyboard
    u16 maxScancode = platform.inputKeyboardGetMaxScancode();
    memory.copy(inputState.keyboard.previousState, inputState.keyboard.currentState, maxScancode);
    // Mouse
    inputState.mouse.previousButtons = inputState.mouse.currentButtons;
    inputState.mouse.scrollWheel = Vec2(0, 0);
    // Controller
    i16 maxControllerButton = static_cast<i16>(ControllerButton::Max);
    memory.copy(inputState.controller.previousButtons, inputState.controller.currentButtons, maxControllerButton);
}

void InputManager::update() {
    auto& platform = Locator::platform();
    // Mouse
    i32 x = 0, y = 0;
    if (inputState.mouse.isRelativeMode) {
        inputState.mouse.currentButtons = platform.inputMouseGetRelativeState(x, y);
    } else {
        inputState.mouse.currentButtons = platform.inputMouseGetState(x, y);
    }

    inputState.mouse.position.x = static_cast<float>(x);
    inputState.mouse.position.y = static_cast<float>(y);
    if (!inputState.mouse.isRelativeMode) {
        // We want mouse coordinates to be between minus half the screen's size and plus half the screen size
        inputState.mouse.position.x -= static_cast<float>(windowWidth) * 0.5f;
        inputState.mouse.position.y = static_cast<float>(windowHeight) * 0.5f - inputState.mouse.position.y;
    }

    // Controller
    // Buttons
    i16 controllerMaxButton = static_cast<i16>(ControllerButton::Max);
    for (i32 i = 0; i < controllerMaxButton; i++) {
        inputState.controller.currentButtons[i] = platform.inputControllerGetButton(controllerPtr, SDL_GameControllerButton(i));
    }

    // Triggers
    inputState.controller.leftTrigger = filter1D(
            platform.inputControllerGetAxis(controllerPtr, ControllerAxis::TriggerLeft));
    inputState.controller.rightTrigger = filter1D(
            platform.inputControllerGetAxis(controllerPtr, ControllerAxis::TriggerRight));

    // Sticks
    x = platform.inputControllerGetAxis(controllerPtr, ControllerAxis::LeftX);
    y = -platform.inputControllerGetAxis(controllerPtr, ControllerAxis::LeftY);
    inputState.controller.leftStick = filter2D(x, y);

    x = platform.inputControllerGetAxis(controllerPtr, ControllerAxis::RightX);
    y = -platform.inputControllerGetAxis(controllerPtr, ControllerAxis::RightY);
    inputState.controller.rightStick = filter2D(x, y);
}

void InputManager::setMouseCursor(bool isCursorDisplayedP) {
    isCursorDisplayed = isCursorDisplayedP;
    Locator::platform().inputMouseShowCursor(isCursorDisplayed);
}

void InputManager::setMouseRelativeMode(bool isMouseRelativeOnP) {
    Locator::platform().inputMouseSetRelativeMode(isMouseRelativeOnP);
    inputState.mouse.isRelativeMode = isMouseRelativeOnP;
}

float InputManager::filter1D(i32 input) {
    const i32 deadZone = CONTROLLER_DEAD_ZONE_1D;
    const i32 maxValue = CONTROLLER_MAX_VALUE;
    f32 retVal = 0.0f;

    i32 absValue = input > 0 ? input : -input;
    if (absValue > deadZone) {
        // Compute fractional value between dead zone and max value
        retVal = static_cast<float>(absValue - deadZone) / (maxValue - deadZone);
        // Make sure sign matches original value
        retVal = input > 0 ? retVal : -1.0f * retVal;
        // Clamp between -1.0f and 1.0f
        retVal = math::clamp(retVal, -1.0f, 1.0f);
    }

    return retVal;
}

Vec2 InputManager::filter2D(i32 inputX, i32 inputY) {
    const float deadZone = CONTROLLER_DEAD_ZONE_2D;
    const float maxValue = CONTROLLER_MAX_VALUE;

    Vec2 dir;
    dir.x = static_cast<f32>(inputX);
    dir.y = static_cast<f32>(inputY);
    float length = static_cast<f32>(dir.length());

    // If length < deadZone, should be no input
    if (length < deadZone) {
        dir = Vec2(0, 0);
    } else {
        // Calculate fractional value between dead zone and max value circles
        float f = (length - deadZone) / (maxValue - deadZone);
        // Clamp f between 0.0f and 1.0f
        f = math::clamp(f, 0.0f, 1.0f);
        // Normalize the vector, and then scale it to the fractional value
        dir *= f / length;
    }

    return dir;
}
