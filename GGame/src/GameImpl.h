//
// Created by gaetz on 08/06/2021.
//
#include <Defines.h>
#include <Game.h>

#ifndef GAMEIMPL_H
#define GAMEIMPL_H

namespace game {

    class GameImpl : public Game {
    public:

        static GameImpl& instance() {
            static GameImpl gameInstance;
            return gameInstance;
        }

        ~GameImpl() override = default;
        void load() override;
        void update(u32 dt) override;
        void draw() override;
        void close() override;

    private:
        GameImpl() = default;
    };

}
#endif //GAMEIMPL_H
