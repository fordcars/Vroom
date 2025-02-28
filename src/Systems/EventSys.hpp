#pragma once

class EventSys {
public:
    static EventSys& get();
    EventSys() = default;
    bool handleEvents();

private:
    EventSys(const EventSys&) = delete;
    EventSys& operator=(const EventSys&) = delete;
    EventSys(EventSys&&) = delete;
    EventSys& operator=(EventSys&&) = delete;
};