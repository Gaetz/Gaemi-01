//
// Created by gaetz on 08/06/2021.
//
#include <Game.h>
#include "Defines.h"

#ifndef GAMEIMPL_H
#define GAMEIMPL_H

namespace game {

    class GameImpl : public Game {
    public:
        ~GameImpl() override = default;

        static GameImpl& instance() {
            static GameImpl gameInstance;
            return gameInstance;
        }

        void load() override;
        void update(u32 dt) override;
        void draw() override;
        void cleanup() override;

    private:
        GameImpl() = default;
    };

}
#endif //GAMEIMPL_H
