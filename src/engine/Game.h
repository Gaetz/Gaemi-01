//
// Created by gaetz on 08/06/2021.
//

#ifndef GAME_H
#define GAME_H

class Game
{
public:

    bool isInitialized{ false };
    int frameNumber {0};
    VkExtent2D windowExtent{ 1700 , 900 };
    struct SDL_Window* window{ nullptr };

    // Initializes everything in the engine
    void init();

    // Shuts down the engine
    void cleanup();

    // Draw loop
    void draw();

    // Run main loop
    void run();
};

#endif //GAME_H
