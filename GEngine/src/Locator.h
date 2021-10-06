//
// Created by gaetz on 06/10/2021.
//

#ifndef LOCATOR_H
#define LOCATOR_H

#include "platforms/Platform.h"
#include "Memory.h"
#include "Events.h"

using engine::Memory;
using engine::NullMemory;
using engine::platforms::Platform;
using engine::platforms::NullPlatform;
using engine::Events;
using engine::NullEvents;

/**
 * Service locator pattern, to access application-wide services. Better than a singleton.
 * Available services: platform, memory, events.
 *
 * Usage: Locator::service().expressionNeeded
 */
class Locator {
public:
    static void init();

    GAPI static Platform& platform() { return *platformService; };
    GAPI static Memory& memory() { return *memoryService; };
    GAPI static Events& events() { return *eventsService; };

    static void provide(Memory* service);
    static void provide(Platform* service);
    static void provide(Events* service);

private:
    static Platform* platformService;
    static NullPlatform nullPlatformService;

    static Memory* memoryService;
    static NullMemory nullMemoryService;

    static Events* eventsService;
    static NullEvents nullEventsService;
};

#endif //LOCATOR_H
