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
    auto& subscriptions = state[eventCode].subscriptions;

    // Check for duplicates
    for (auto& subscription : subscriptions) {
        if (subscription.listener == listener) {
            return false;
        }
    }

    // Proceed with subscription
    Subscription subscription;
    subscription.listener = listener;
    subscription.callback = onEvent;
    subscriptions.push_back(subscription);
    return true;
}

bool engine::EventSystem::unsubscribe(engine::EventCode code, void *listener, engine::EventCallback* onEvent) {
    u16 eventCode = static_cast<u16>(code);
    auto& subs = state[eventCode].subscriptions;
    if (subs.empty()) return false;

    bool isRemoved = false;
    subs.erase(std::remove_if(subs.begin(), subs.end(), [=, &isRemoved](const auto& sub){
        if(sub.listener == listener && sub.callback == onEvent) {
            isRemoved = true;
            return true;
        }
        return false;
    }), subs.end());
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
