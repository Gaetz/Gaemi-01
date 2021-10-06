//
// Created by gaetz on 04/10/2021.
//

#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include "Defines.h"
#include "Events.h"

namespace engine {

    // Not multithreaded yet, no priority system yet
    class EventManager : public Events {
    public:
        EventManager() = default;
        ~EventManager() = default;

        bool init();
        void close();

        /**
         * Subscribe to listen events with the provided code.
         * Events with duplicate listener/callback combo will not be subscribed
         * and cause this function to return false.
         * @param code Event code to listen for.
         * @param listener Listener instance reference.
         * @param onEvent Callback function to be invoked when the event code is fired.
         * @returns true when the event is successfully subscribed, false otherwise.
         */
        GAPI bool subscribe(EventCode code, void *listener, EventCallback* onEvent);

        /**
         * Unsubscribe from to listening to events with the provided code.
         * If no subscription is found, this function to return false.
         * @param code Event code to listen for.
         * @param listener Listener instance reference.
         * @param onEvent Callback function to be unregistred from.
         * @returns true when the event is successfully unsubscribed, false otherwise.
         */
        GAPI bool unsubscribe(EventCode code, void *listener, EventCallback* onEvent);

        /**
         * Fire an event to listeners of the given code. If an event handler returns true,
         * the event is considered handled and is not passed on to any more listeners.
         * @param code Code of the event to fire.
         * @param sender Reference to the sender.
         * @param context Event data
         * @return True if handled, otherwise false.
         */
        GAPI bool fire(EventCode code, void *sender, EventContext context);

        static array<EventCodeEntry, MAX_EVENT_CODE> state;
    private:
        bool isInitialized { false };

    };
}

#endif //EVENTMANAGER_H
