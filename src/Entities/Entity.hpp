#pragma once
#include <tuple>

template<typename... ComponentTs>
class Entity {
public:
    std::tuple<ComponentTs...> mComponents = {};

    template<typename ComponentT>
    ComponentT& get() {
        return std::get<ComponentT>(mComponents);
    }

    template<typename ComponentT>
    const ComponentT& get() const {
        return std::get<ComponentT>(mComponents);
    }

    // Useful for structured bindings when iterating.
    // Returns tuple of references to components.
    auto getComponents() {
        return std::tie(std::get<ComponentTs>(mComponents)...);
    }

    auto getComponents() const {
        return std::tie(std::get<ComponentTs>(mComponents)...);
    }
};