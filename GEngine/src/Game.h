//
// Created by gaetz on 25/09/2021.
//

#ifndef GAME_H
#define GAME_H

#include "Defines.h"
#include "input/InputSystem.h"

using engine::input::InputState;

namespace game {
    class Game {
    public:
        virtual ~Game() = default;
        virtual void load() = 0;
        virtual void update(u32 dt) = 0;
        virtual void draw() = 0;
        virtual void close() = 0;

        void setInputState(const InputState& inputStateP) {
            // Implicit copy
            inputState = inputStateP;
        }

    protected:
        InputState inputState;
    };
}

#endif //GAME_H
