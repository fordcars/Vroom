#pragma once
#include <tuple>

template<typename... ComponentTs>
class Entity {
public:
    std::tuple<ComponentTs...> mComponents;

    template<typename ComponentT>
    ComponentT& get() {
        return std::get<ComponentT>(mComponents);
    }

    // Useful for structured bindings when iterating
    auto getComponents() {
        return std::tie(std::get<ComponentTs>(mComponents)...);
    }
};