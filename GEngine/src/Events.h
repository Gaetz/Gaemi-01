//
// Created by gaetz on 04/10/2021.
//

#ifndef EVENTS_H
#define EVENTS_H

#include <functional>
#include <array>
using std::array;
#include <vector>
using std::vector;

#include "Log.h"

namespace engine {

    enum class EventCode {
        ApplicationQuit,
        WindowResized,

        MaxEventCode
    };

    constexpr u16 MAX_EVENT_CODE = 16384;

    struct EventContext {
        union {
            array<i64, 2> i64;
            array<u64, 2> u64;
            array<f64, 2> f64;

            array<i32, 4> i32;
            array<u32, 4> u32;
            array<f32, 4> f32;

            array<i16, 8> i16;
            array<u16, 8> u16;

            array<i8, 16> i8;
            array<u8, 16> u8;
            array<char, 16> c;
        } data;
    };

    using EventCallback = std::function<bool(engine::EventCode code, void *sender, void *listenerInstance,
                                             engine::EventContext data)>;

    struct Subscription {
        void* listener { nullptr };
        EventCallback* callback { nullptr };

        bool operator==(const Subscription& other) {
            return listener == other.listener && callback == other.callback;
        }
    };

    struct EventCodeEntry {
        vector<Subscription> subscriptions {};
    };

    class Events {
    public:
        /**
         * Subscribe to listen events with the provided code.
         * Events with duplicate listener/callback combo will not be subscribed
         * and cause this function to return false.
         * @param code Event code to listen for.
         * @param listener Listener instance reference.
         * @param onEvent Callback function to be invoked when the event code is fired.
         * @returns true when the event is successfully subscribed, false otherwise.
         */
        GAPI virtual bool subscribe(EventCode code, void *listener, EventCallback* onEvent) = 0;

        /**
         * Unsubscribe from to listening to events with the provided code.
         * If no subscription is found, this function to return false.
         * @param code Event code to listen for.
         * @param listener Listener instance reference.
         * @param onEvent Callback function to be unregistred from.
         * @returns true when the event is successfully unsubscribed, false otherwise.
         */
        GAPI virtual bool unsubscribe(EventCode code, void *listener, EventCallback* onEvent) = 0;

        /**
         * Fire an event to listeners of the given code. If an event handler returns true,
         * the event is considered handled and is not passed on to any more listeners.
         * @param code Code of the event to fire.
         * @param sender Reference to the sender.
         * @param context Event data
         * @return True if handled, otherwise false.
         */
        GAPI virtual bool fire(EventCode code, void *sender, EventContext context) = 0;
    };

    class NullEvents : public Events {
    private:
        void placeholderMessage() {
            LOG(LogLevel::Warning) << "Usage of placeholder memory service.";
        }

    public:
        GAPI bool subscribe(EventCode code, void *listener, EventCallback* onEvent) override { placeholderMessage(); return false; }
        GAPI bool unsubscribe(EventCode code, void *listener, EventCallback* onEvent) override { placeholderMessage(); return false; }
        GAPI bool fire(EventCode code, void *sender, EventContext context) override { placeholderMessage(); return false; }
    };
}

#endif //EVENTS_H
