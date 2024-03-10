#pragma once
#include "SystemEntities.hpp"

// All entities with ComponentTs will be accessible through
// the iterable returned from getEntities().
template<typename... ComponentTs>
class System {
protected:
    SystemEntities<ComponentTs...>& getEntities() {
        return mEntities;
    }

private:
    SystemEntities<ComponentTs...> mEntities;
};
