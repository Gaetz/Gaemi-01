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
        GAPI u32 computeDeltaTime(u64 absoluteTime);

        // Wait if the game run faster than the decided FPS
        void delayTime(const Engine& engine, u64 absoluteTime);

    private:
        const static u32 FPS = 60;
        const static u32 frameDelay = 1000 / FPS;

        // Time in milliseconds when frame starts
        u64 frameStart;

        // Last frame start time in milliseconds
        u64 lastFrame;

        // Time it tooks to run the loop. Used to cap framerate.
        u32 frameTime;
    };
}
#endif