#ifndef TIMER_H
#define TIMER_H

#include "Defines.h"

namespace engine {

    class Engine;

// Hold time related functions.
// In charge of computing the delta time and
// ensure smooth game ticking.
    class Timer {
    public:
        GAPI Timer();

        GAPI virtual ~Timer();

        // Compute delta time as the number of milliseconds since last frame
        GAPI unsigned int computeDeltaTime(u32 absoluteTime);

        // Wait if the game run faster than the decided FPS
        void delayTime(const Engine& engine, u32 absoluteTime);

    private:
        const static int FPS = 60;
        const static int frameDelay = 1000 / FPS;

        // Time in milliseconds when frame starts
        unsigned int frameStart;

        // Last frame start time in milliseconds
        unsigned int lastFrame;

        // Time it tooks to run the loop. Used to cap framerate.
        unsigned int frameTime;
    };
}
#endif