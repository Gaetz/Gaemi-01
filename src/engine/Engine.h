//
// Created by gaetz on 08/06/2021.
//

#ifndef ENGINE_H
#define ENGINE_H


#include "Types.h"
#include "Window.h"
#include "Timer.h"

class Engine
{
public:

    bool isInitialized{ false };
    int frameNumber {0};
    VkExtent2D windowExtent{ 1280 , 720 };
    Window window {"Gaemi-01"};

    // Initializes everything in the engine
    void init();

    // Shuts down the engine
    void cleanup();

    // Draw loop
    void draw();

    // Run main loop
    void run();
};

#endif //ENGINE_H
