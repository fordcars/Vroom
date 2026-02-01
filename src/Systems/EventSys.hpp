#pragma once

class EventSys {
public:
    static EventSys &get();
    EventSys() = default;
    EventSys(const EventSys &) = delete;
    EventSys &operator=(const EventSys &) = delete;
    EventSys(EventSys &&) = delete;
    EventSys &operator=(EventSys &&) = delete;

    bool handleEvents();
};