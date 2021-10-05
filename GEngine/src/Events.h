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

namespace engine {

    enum class EventCode {
        ApplicationQuit,
        WindowResized,

        MaxEventCode
    };

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

    constexpr u16 MAX_EVENT_CODE = 16384;
}

#endif //EVENTS_H
