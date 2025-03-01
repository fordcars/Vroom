#pragma once
#include <tuple>

#include "Utils/TupleUtils.hpp"

template <typename... ComponentTs>
class Entity {
public:
    std::tuple<ComponentTs...> mComponents = {};

    template <typename ComponentT>
    decltype(auto) get() {
        return Utils::OptionalTupleGetter<ComponentT>::get(mComponents);
    }

    template <typename ComponentT>
    decltype(auto) get() const {
        return Utils::OptionalTupleGetter<ComponentT>::get(mComponents);
    }

    // Useful for structured bindings.
    // Returns tuple of references to components.
    auto getComponents() {
        return std::tie(Utils::OptionalTupleGetter<ComponentTs>::get(mComponents)...);
    }
    auto getComponents() const {
        return std::tie(Utils::OptionalTupleGetter<ComponentTs>::get(mComponents)...);
    }
};