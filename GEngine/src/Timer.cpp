#include "Timer.h"
#include "Engine.h"

using engine::Timer;

Timer::Timer() : frameStart(0), lastFrame(0), frameTime(0)
{
}

Timer::~Timer()
{
}

unsigned int Timer::computeDeltaTime(u32 absoluteTime)
{
    frameStart = absoluteTime;
    unsigned int dt = frameStart - lastFrame;
    lastFrame = frameStart;
    return dt;
}

void Timer::delayTime(const Engine& engine, u32 absoluteTime)
{
    frameTime = absoluteTime - frameStart;
    if (frameTime < frameDelay) {
        engine.sleep(frameDelay - frameTime);
    }
}

