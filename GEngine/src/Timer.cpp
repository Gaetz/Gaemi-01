#include "Timer.h"
#include "Locator.h"

using engine::Timer;

Timer::Timer() : frameStart(0), lastFrame(0), frameTime(0)
{
}

Timer::~Timer()
{
}

u32 Timer::computeDeltaTime(u64 absoluteTime)
{
    frameStart = absoluteTime;
    u32 dt = static_cast<u32>(frameStart - lastFrame);
    lastFrame = frameStart;
    return dt;
}

void Timer::delayTime(const Engine& engine, u64 absoluteTime)
{
    frameTime = static_cast<u32>(absoluteTime - frameStart);
    if (frameTime < frameDelay) {
        Locator::platform().sleep(static_cast<u32>(frameDelay - frameTime));
    }
}

