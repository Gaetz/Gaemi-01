//
// Created by gaetz on 06/10/2021.
//

#ifndef LOCATOR_H
#define LOCATOR_H

#include "platforms/Platform.h"
#include "mem/Memory.h"
#include "Events.h"
#include "asset/Assets.h"
#include "render/vk/GameObject.h"

using engine::mem::Memory;
using engine::mem::NullMemory;
using engine::platforms::Platform;
using engine::platforms::NullPlatform;
using engine::Events;
using engine::NullEvents;
using engine::asset::Assets;
using engine::asset::NullAssets;
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
    GAPI static engine::mem::Memory& memory() { return *memoryService; };
    GAPI static Events& events() { return *eventsService; };
    GAPI static Assets& assets() { return *assetsService; };

    static void provide(engine::mem::Memory* service);
    static void provide(Platform* service);
    static void provide(Events* service);
    static void provide(Assets* service);

private:
    static Platform* platformService;
    static NullPlatform nullPlatformService;

    static engine::mem::Memory* memoryService;
    static engine::mem::NullMemory nullMemoryService;

    static Events* eventsService;
    static NullEvents nullEventsService;

    static Assets* assetsService;
    static NullAssets nullAssetsService;
};

#endif //LOCATOR_H
