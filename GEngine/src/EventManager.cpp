//
// Created by gaetz on 04/10/2021.
//

#include <algorithm>
#include "EventManager.h"
#include "Engine.h"
#include "Locator.h"

using engine::EventManager;

array<engine::EventCodeEntry, engine::MAX_EVENT_CODE> EventManager::state {};

bool EventManager::init() {
    if (isInitialized) return false;
    Locator::memory().zero(&state, sizeof(state));
    isInitialized = true;
    Locator::provide(this);
    return true;
}

void EventManager::close() {
    for (u16 i = 0; i < MAX_EVENT_CODE; ++i) {
        if(!state[i].subscriptions.empty()) {
            state[i].subscriptions.clear();
        }
    }
}

bool engine::EventManager::subscribe(engine::EventCode code, void *listener, engine::EventCallback* onEvent) {
    u16 eventCode = static_cast<u16>(code);

    // Check for duplicates
    for (auto& subscription : state[eventCode].subscriptions) {
        if (subscription.listener == listener) {
            return false;
        }
    }

    // Proceed with subscription
    Subscription subscription;
    subscription.listener = listener;
    subscription.callback = onEvent;
    state[eventCode].subscriptions.push_back(subscription);
    return true;
}

bool engine::EventManager::unsubscribe(engine::EventCode code, void *listener, engine::EventCallback* onEvent) {
    u16 eventCode = static_cast<u16>(code);
    auto& subs = state[eventCode].subscriptions;
    if (subs.empty()) return false;

    bool isRemoved = false;
    auto i = 0;
    while(i < subs.size()) {
        if (subs[i].listener != listener || subs[i].callback != onEvent) {
            ++i;
            continue;
        } else {
            isRemoved = true;
            break;
        }
    }

    if(isRemoved) {
        auto unused = std::remove(subs.begin(), subs.end(), subs[i]);
    }

    return isRemoved;
}

bool engine::EventManager::fire(engine::EventCode code, void *sender, engine::EventContext context) {
    u16 eventCode = static_cast<u16>(code);
    auto& subs = state[eventCode].subscriptions;
    if (subs.empty()) return false;

    return std::any_of(subs.begin(), subs.end(), [=, &context](const auto& sub) {
        return (*(sub.callback))(code, sender, sub.listener, context);
    });
}
