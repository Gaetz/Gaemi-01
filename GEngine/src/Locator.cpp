//
// Created by gaetz on 06/10/2021.
//

#include "Locator.h"
#include "Memory.h"

Memory* Locator::memoryService {nullptr};
Platform* Locator::platformService {nullptr};
Events* Locator::eventsService {nullptr};
Assets* Locator::assetsService {nullptr};

NullPlatform Locator::nullPlatformService {};
NullMemory Locator::nullMemoryService {};
NullEvents Locator::nullEventsService {};
NullAssets Locator::nullAssetsService {};

void Locator::init() {
    memoryService = &nullMemoryService;
    platformService = &nullPlatformService;
    eventsService = &nullEventsService;
    assetsService = &nullAssetsService;
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

void Locator::provide(Assets *service) {
    if (service == nullptr) service = &nullAssetsService;
    assetsService = service;
}
