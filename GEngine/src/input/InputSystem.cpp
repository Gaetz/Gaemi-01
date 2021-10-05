//
// Created by gaetz on 12/06/2021.
//

#include "InputSystem.h"
#include "../math/Functions.h"
#include "../Engine.h"
#include "../Log.h"

using engine::input::InputSystem;

InputSystem::InputSystem(u32 windowWidthP, u32 windowHeightP) :
        windowWidth {windowWidthP},
        windowHeight {windowHeightP} {

}

bool InputSystem::init() {
    EngineState& engineState = Engine::getState();
    // Keyboard
    // Assign current state pointer
    inputState.keyboard.currentState = engineState.platform->inputKeyboardGetState(nullptr);
    // Clear previous state memory
    u16 maxScancode = engineState.platform->inputKeyboardGetMaxScancode();
    engineState.memoryManager.set(inputState.keyboard.previousState, 0, maxScancode);

    // Mouse (just set everything to 0)
    inputState.mouse.currentButtons = 0;
    inputState.mouse.previousButtons = 0;

    // Get the connected controllerPtr, if it exists
    controllerPtr = engineState.platform->inputControllerOpen(0);
    // Initialize controllerPtr state
    inputState.controller.isConnected = (controllerPtr != nullptr);
    i16 maxControllerButton = static_cast<i16>(ControllerButton::Max);
    engineState.memoryManager.set(inputState.controller.currentButtons, 0, maxControllerButton);
    engineState.memoryManager.set(inputState.controller.previousButtons, 0, maxControllerButton);

    return true;
}

void InputSystem::cleanup() {
    if (controllerPtr != nullptr) {
        Engine::getState().platform->inputControllerClose(controllerPtr);
    }
}

void InputSystem::processEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            Engine::getState().eventSystem.fire(EventCode::ApplicationQuit, nullptr, {});
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

void InputSystem::preUpdate() {
    EngineState& engineState = Engine::getState();
    // Copy current state to previous
    // Keyboard
    u16 maxScancode = engineState.platform->inputKeyboardGetMaxScancode();
    engineState.memoryManager.copy(inputState.keyboard.previousState, inputState.keyboard.currentState, maxScancode);
    // Mouse
    inputState.mouse.previousButtons = inputState.mouse.currentButtons;
    inputState.mouse.scrollWheel = Vec2(0, 0);
    // Controller
    i16 maxControllerButton = static_cast<i16>(ControllerButton::Max);
    engineState.memoryManager.copy(inputState.controller.previousButtons, inputState.controller.currentButtons, maxControllerButton);
}

void InputSystem::update() {
    EngineState& engineState = Engine::getState();
    // Mouse
    i32 x = 0, y = 0;
    if (inputState.mouse.isRelativeMode) {
        inputState.mouse.currentButtons = engineState.platform->inputMouseGetRelativeState(x, y);
    } else {
        inputState.mouse.currentButtons = engineState.platform->inputMouseGetState(x, y);
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
        inputState.controller.currentButtons[i] = engineState.platform->inputControllerGetButton(controllerPtr, SDL_GameControllerButton(i));
    }

    // Triggers
    inputState.controller.leftTrigger = filter1D(
            engineState.platform->inputControllerGetAxis(controllerPtr, ControllerAxis::TriggerLeft));
    inputState.controller.rightTrigger = filter1D(
            engineState.platform->inputControllerGetAxis(controllerPtr, ControllerAxis::TriggerRight));

    // Sticks
    x = engineState.platform->inputControllerGetAxis(controllerPtr, ControllerAxis::LeftX);
    y = -engineState.platform->inputControllerGetAxis(controllerPtr, ControllerAxis::LeftY);
    inputState.controller.leftStick = filter2D(x, y);

    x = engineState.platform->inputControllerGetAxis(controllerPtr, ControllerAxis::RightX);
    y = -engineState.platform->inputControllerGetAxis(controllerPtr, ControllerAxis::RightY);
    inputState.controller.rightStick = filter2D(x, y);
}

void InputSystem::setMouseCursor(bool isCursorDisplayedP) {
    isCursorDisplayed = isCursorDisplayedP;
    Engine::getState().platform->inputMouseShowCursor(isCursorDisplayed);
}

void InputSystem::setMouseRelativeMode(bool isMouseRelativeOnP) {
    Engine::getState().platform->inputMouseSetRelativeMode(isMouseRelativeOnP);
    inputState.mouse.isRelativeMode = isMouseRelativeOnP;
}

float InputSystem::filter1D(i32 input) {
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

Vec2 InputSystem::filter2D(i32 inputX, i32 inputY) {
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
