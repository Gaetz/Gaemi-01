//
// Created by gaetz on 04/10/2021.
//

#include <algorithm>
#include "EventSystem.h"
#include "Engine.h"

using engine::EventSystem;

array<engine::EventCodeEntry, engine::MAX_EVENT_CODE> EventSystem::state {};

bool EventSystem::init() {
    if (isInitialized) return false;
    Engine::getState().memoryManager.zero(&state, sizeof(state));
    isInitialized = true;
    return true;
}

void EventSystem::close() {
    for (u16 i = 0; i < MAX_EVENT_CODE; ++i) {
        if(!state[i].subscriptions.empty()) {
            state[i].subscriptions.clear();
        }
    }
}

bool engine::EventSystem::subscribe(engine::EventCode code, void *listener, engine::EventCallback* onEvent) {
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

bool engine::EventSystem::unsubscribe(engine::EventCode code, void *listener, engine::EventCallback* onEvent) {
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

bool engine::EventSystem::fire(engine::EventCode code, void *sender, engine::EventContext context) {
    u16 eventCode = static_cast<u16>(code);
    auto& subs = state[eventCode].subscriptions;
    if (subs.empty()) return false;

    return std::any_of(subs.begin(), subs.end(), [=, &context](const auto& sub) {
        return (*(sub.callback))(code, sender, sub.listener, context);
    });
}
