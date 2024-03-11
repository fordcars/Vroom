#pragma once
#include <tuple>
#include <optional>
#include <type_traits>
#include <cassert>
#include "Entities/EntityRegistry.hpp"

// Helper classes for checking if tuple has type
// https://stackoverflow.com/a/25958302/6222104
namespace SystemEntitiesTupleHelpers {
    template <typename T, typename Tuple>
    struct has_type;

    template <typename T>
    struct has_type<T, std::tuple<>> : std::false_type {};

    template <typename T, typename U, typename... Ts>
    struct has_type<T, std::tuple<U, Ts...>> : has_type<T, std::tuple<Ts...>> {};

    template <typename T, typename... Ts>
    struct has_type<T, std::tuple<T, Ts...>> : std::true_type {};

    template <typename T, typename Tuple>
    using tuple_contains_type = typename has_type<T, Tuple>::type;
}

//// TODO: Change copies to references
// Get tuple of ComponentTs values from EntityT
// Ex: If the entity has components A, B, C, D, and
// ComponentTs are A, D, C, we want to return a
// std::tuple<A&, D&, C&>.
template<typename EntityT>
class EntityComponentFilter {
public:
    EntityComponentFilter(EntityT& entity) : mEntity(entity) {}

    // Base-case: one component
    template<typename ComponentT>
    std::tuple<ComponentT> getTuple() {
        return std::tuple<ComponentT>(mEntity.get<ComponentT>()); // std::tie
    }

    // https://stackoverflow.com/a/31356705/6222104
    template<typename FirstComponentT, typename SecondComponentT, typename... ComponentTs>
    std::tuple<FirstComponentT, ComponentTs...> getTuple() {
        return std::tuple_cat(
            std::tuple<FirstComponentT, SecondComponentT>(
                mEntity.get<FirstComponentT>(),
                mEntity.get<SecondComponentT>()
            ),
            getTuple<ComponentTs...>(mEntity)
        );
    }

private:
    EntityT& mEntity;
};

namespace IsValidEntityInternal {
    // Forward decl
    template<typename EntityT, typename... ComponentTs>
    struct IsValidEntity_Type;

    // Recursive case: check if the first type matches, or check the rest of the tuple
    template<typename EntityT, typename FirstComponentT, typename... RestComponentTs>
    struct IsValidEntity_Type<EntityT, FirstComponentT, RestComponentTs...> :
        std::conditional<SystemEntitiesTupleHelpers::tuple_contains_type<FirstComponentT, decltype(EntityT::mComponents)>::value,
            IsValidEntity_Type<EntityT, RestComponentTs...>,
            std::false_type
            >::type {};

    // Base case: empty ComponentTs (valid EntityT)
    template<typename EntityT, typename... ComponentTs>
    struct IsValidEntity_Type : std::true_type {};
}

template<typename EntityT, typename... ComponentTs>
concept IsValidEntity = IsValidEntityInternal::IsValidEntity_Type<EntityT, ComponentTs...>::value;

// Helper class for getting the next entity with the specified components
template<typename... ComponentTs>
class SystemEntitiesGetter {
public:
    // Find next valid entity
    // Return empty value if there is no next entity
    template<typename... EntityTs>
    std::optional<std::tuple<ComponentTs...>> getNextEntity(const EntityRegistry<EntityTs...>&) {
        std::optional<std::tuple<ComponentTs...>> returnValue;
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
    
    template<typename EntityT>
    void getNextEntityOfEntityT(std::optional<std::tuple<ComponentTs...>>& returnValue)
        requires IsValidEntity<EntityT, ComponentTs...> {
        if(mCurrentEntityVector == nullptr) {
            // This is the first entity being traversed in this EntityT
            mCurrentEntityVector = &(EntityT::instances);
            mIndex = 0;
        }

        if(mCurrentEntityVector == &(EntityT::instances) && !returnValue.has_value()) {
            // The is the current EntityT being traversed
            if(mIndex < EntityT::instances.size()) {
                returnValue = EntityComponentFilter(EntityT::instances[mIndex]).getTuple<ComponentTs...>();
                ++mIndex;
            }

            if(mIndex >= EntityT::instances.size()) {
                // Done traversing this EntityT, go to next one!
                mCurrentEntityVector = nullptr;
            }
        }
    }

    // SFINAE
    template<typename EntityT>
    void getNextEntityOfEntityT(std::optional<std::tuple<ComponentTs...>>& returnValue)
        requires (!IsValidEntity<EntityT, ComponentTs...>) {}
};

// Iterable class for getting the current entities which
// have the specified components.
template<typename... ComponentTs>
class SystemEntities {
public:
    class Iterator {
    public:
        Iterator(bool isEnd = false) : mIsEnd(isEnd) {
            if(!mIsEnd) {
                mCurrentEntity = mEntitiesGetter.getNextEntity(mEntityRegistry);
                if(!mCurrentEntity.has_value()) mIsEnd = true;
            }
        }

        std::tuple<ComponentTs...>& operator*() {
            return mCurrentEntity.value();
        }

        Iterator& operator++() {
            mCurrentEntity = mEntitiesGetter.getNextEntity(mEntityRegistry);
            if(!mCurrentEntity.has_value()) mIsEnd = true;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return mIsEnd != other.mIsEnd;
        }
        
    private:
        bool mIsEnd;
        EntityRegistryDefinition mEntityRegistry; // This is in reality an empty class
        SystemEntitiesGetter<ComponentTs...> mEntitiesGetter;
        std::optional<std::tuple<ComponentTs...>> mCurrentEntity;
    };

    Iterator begin() {
        return {};
    }

    Iterator end() {
        return {true};
    }
};