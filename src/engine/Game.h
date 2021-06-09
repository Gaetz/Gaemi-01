//
// Created by gaetz on 08/06/2021.
//

#ifndef GAME_H
#define GAME_H

class Game {
public:
    Game();
    ~Game();

    void load();
    void handleInputs();
    void update(unsigned int dt);
    void draw();
    void cleanup();

    bool isRunning;
};

#endif //GAME_H
