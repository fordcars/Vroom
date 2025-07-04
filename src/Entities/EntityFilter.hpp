// This file deals with Entities, Systems and Components.
// For more information: https://en.wikipedia.org/wiki/Entity_component_system
// Essentially, ComponentFilter is an iterable class, which will "iterate" through
// all entity types registered in EntityRegistry, but will only return entity
// types/instances which have at least the specified components.
//
// Example usage:
// EntityFilter<PositionComp, RenderableComp> filter;
// for(auto& [position, renderable] : filter)
//     // Do something with entity

#pragma once
#include <cassert>
#include <optional>
#include <tuple>
#include <type_traits>

#include "Entities/Entity.hpp"
#include "Entities/EntityRegistry.hpp"
#include "Utils/TupleUtils.hpp"

namespace EntityFilterInternal {

template <typename... ComponentTs>
using ReturnedComponentsTuple =
    std::tuple<decltype(std::declval<Utils::OptionalTupleGetter<ComponentTs>>().get(
        std::declval<std::tuple<>&>()))...>;

// Get tuple of ComponentTs values from EntityT
// Ex: If the entity has components A, B, C, D, and ComponentTs are A, D, C,
// we want to return std::tuple<A&, D&, C&>.
//
// We also handle optional tuples, so if the entity has components A, B, C, D,
// and ComponentTs are A, D, std::optional<E>, we want to
// return std::tuple<A&, D&, std::optional<std::reference_wrapper<E>>.
// This logic is done in Entity::get<std::optional<E>>().
template <typename EntityT>
class ComponentTuple {
private:
    EntityT& mEntity;

public:
    ComponentTuple(EntityT& entity) : mEntity(entity) {}

    template <typename... ComponentTs>
    ReturnedComponentsTuple<ComponentTs...> getTuple() {
        return {mEntity.get<ComponentTs>()...};
    }
};

namespace IsValidEntityInternal {
// Forward decl
template <typename EntityT, typename... ComponentTs>
struct IsValidEntity_Type;

// Recursive case: check if the first type matches, or check the rest of the tuple
template <typename EntityT, typename FirstComponentT, typename... RestComponentTs>
struct IsValidEntity_Type<EntityT, FirstComponentT, RestComponentTs...>
    : std::conditional<Utils::TupleContainsType<
                           FirstComponentT, decltype(EntityT::mComponents)>::value,
                       IsValidEntity_Type<EntityT, RestComponentTs...>,
                       std::false_type>::type {};

// Base case: empty ComponentTs (valid EntityT)
template <typename EntityT, typename... ComponentTs>
struct IsValidEntity_Type : std::true_type {};
}  // namespace IsValidEntityInternal

// Is true if EntityT has at least the specified components.
template <typename EntityT, typename... ComponentTs>
concept IsValidEntity =
    IsValidEntityInternal::IsValidEntity_Type<EntityT, ComponentTs...>::value;

// Helper class for getting the next entity with the specified components.
template <typename... ComponentTs>
class EntityGetter {
private:
    // Is an std::optional of std::tuple
    using ReturnT = std::optional<ReturnedComponentsTuple<ComponentTs...>>;

public:
    // Find next valid entity
    // Return empty value if there is no next entity
    template <typename... EntityTs>
    ReturnT getNextEntity(const EntityRegistry<EntityTs...>&) {
        ReturnT returnValue;
        if(mReachedEnd) return {};

        (..., getNextEntityOfEntityT<EntityTs>(returnValue));
        if(mCurrentEntityVector == nullptr) {
            // We reached the end!
            mReachedEnd = true;
        }
        return returnValue;
    }

private:
    // The current vector being traversed, and its index
    void* mCurrentEntityVector = nullptr;
    std::size_t mIndex = 0;
    bool mReachedEnd = false;

    template <typename EntityT>
    void getNextEntityOfEntityT(ReturnT& returnValue)
        requires IsValidEntity<EntityT, ComponentTs...>
    {
        if(mCurrentEntityVector == nullptr) {
            // This is the first entity being traversed in this EntityT
            mCurrentEntityVector = &(EntityT::instances);
            mIndex = 0;
        }

        if(mCurrentEntityVector == &(EntityT::instances) && !returnValue.has_value()) {
            // The is the current EntityT being traversed
            if(mIndex < EntityT::instances.size()) {
                returnValue =
                    ComponentTuple(EntityT::instances[mIndex]).getTuple<ComponentTs...>();
                ++mIndex;
            }

            if(mIndex >= EntityT::instances.size()) {
                // Done traversing this EntityT, go to next one!
                mCurrentEntityVector = nullptr;
            }
        }
    }

    // SFINAE
    template <typename EntityT>
    void getNextEntityOfEntityT(ReturnT& returnValue)
        requires(!IsValidEntity<EntityT, ComponentTs...>)
    {}
};

}  // namespace EntityFilterInternal

// Iterable class for getting the current entities which
// have the specified components.
template <typename... ComponentTs>
class EntityFilter {
public:
    class Iterator {
    public:
        Iterator(bool isEnd = false) : mIsEnd(isEnd) {
            if(!mIsEnd) {
                // It's extremely important to use reset() before assigning a new
                // value to the std::optional (at least on Windows), since without it,
                // it will move-assign the next entity in the previous one for some
                // reason. This is because we are using std::tuples of references.
                mCurrentEntity.reset();
                mCurrentEntity = mEntityGetter.getNextEntity(mEntityRegistry);
                if(!mCurrentEntity.has_value()) mIsEnd = true;
            }
        }

        decltype(auto) operator*() { return mCurrentEntity.value(); }

        Iterator& operator++() {
            mCurrentEntity.reset();
            mCurrentEntity = mEntityGetter.getNextEntity(mEntityRegistry);
            if(!mCurrentEntity.has_value()) mIsEnd = true;
            return *this;
        }

        bool operator!=(const Iterator& other) const { return mIsEnd != other.mIsEnd; }

    private:
        bool mIsEnd = false;
        EntityRegistryDefinition mEntityRegistry;  // This is in reality an empty class
        EntityFilterInternal::EntityGetter<ComponentTs...> mEntityGetter;
        std::optional<EntityFilterInternal::ReturnedComponentsTuple<ComponentTs...>>
            mCurrentEntity;
    };

    Iterator begin() { return {}; }

    Iterator end() { return {true}; }
};