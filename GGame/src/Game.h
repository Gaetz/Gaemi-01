//
// Created by gaetz on 08/06/2021.
//

#ifndef GAME_H
#define GAME_H

namespace game {

class Game {
public:
    Game();

    ~Game();

    void load();

    void processInputs();

    void update(unsigned int dt);

    void draw();

    void cleanup();

    bool isRunning;
};
}
#endif //GAME_H
