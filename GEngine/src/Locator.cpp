//
// Created by gaetz on 06/10/2021.
//

#include "Locator.h"

Memory* Locator::memoryService {nullptr};
Platform* Locator::platformService {nullptr};
Events* Locator::eventsService {nullptr};

NullPlatform Locator::nullPlatformService {};
NullMemory Locator::nullMemoryService {};
NullEvents Locator::nullEventsService {};

void Locator::init() {
    memoryService = &nullMemoryService;
    platformService = &nullPlatformService;
    eventsService = &nullEventsService;
}

void Locator::provide(Memory* service) {
    if (service == nullptr) service = &nullMemoryService;
    memoryService = service;
}

void Locator::provide(Platform* service) {
    if (service == nullptr) service = &nullPlatformService;
    platformService = service;
}

void Locator::provide(Events* service) {
    if (service == nullptr) service = &nullEventsService;
    eventsService = service;
}